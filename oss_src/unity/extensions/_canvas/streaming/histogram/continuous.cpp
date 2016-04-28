#include "continuous.hpp"
#include <parallel/lambda_omp.hpp>

namespace graphlab {
namespace _canvas {
namespace streaming {
namespace histogram {

static inline size_t get_bin_idx(
    flexible_type value,
    double scale_min,
    double scale_max
  ) {
  double range = scale_max - scale_min;
  size_t bin = std::floor(
    ((static_cast<double>(value) - scale_min) / range) *
    static_cast<double>(continuous_result::MAX_BINS)
  );
  if (bin == continuous_result::MAX_BINS) {
    bin -= 1;
  }
  DASSERT_LT(bin, continuous_result::MAX_BINS);
  return bin;
}

static inline double get_value_at_bin(
    double bin_idx,
    double scale_min,
    double scale_max,
    double num_bins
    ) {
  return (
    (bin_idx / num_bins) *
    (scale_max - scale_min)
  ) + scale_min;
}

void continuous_result::init(flexible_type value1, flexible_type value2) {
  // use 2 initial values for both min/max and bin scale value1/value2
  this->init(value1, value2, value1, value2);
}
void continuous_result::init(
  flexible_type value1, flexible_type value2, double scale1, double scale2
) {
  // initialize bins to 0
  for (size_t i=0; i<this->bins.size(); i++) {
    this->bins[i] = 0;
  }

  this->min = std::min(value1, value2);
  this->max = std::max(value1, value2);
  this->scale_min = std::min(scale1, scale2);
  this->scale_max = std::max(scale1, scale2);
  if (this->scale_max == this->scale_min) {
    // make sure they are not the same value
    if(this->scale_max>0){
      this->scale_max *= (1.0+1e-2);
    }else if(this->scale_max<0){
      this->scale_max *= (1.0-1e-2);
    }else{ // scale_max == 0
      this->scale_max += 1e-2;
    }
  }
}

void continuous_result::rescale(double new_min, double new_max) {
  // collapse bins towards the center to expand range by 2x
  while (new_min < scale_min || new_max > scale_max) {
    // first, combine bins next to each other (every other bin)
    for (ssize_t i=(MAX_BINS / 2) - 1; i>0; i-=2) {
      bins[i] += bins[i-1];
    }
    for (size_t i=(MAX_BINS / 2); i<MAX_BINS; i+=2) {
      bins[i] += bins[i+1];
    }
    // then, collapse them inward towards the center
    for (size_t i=0; i<(MAX_BINS/4); i++) {
      bins[(MAX_BINS/2) + i] = bins[(MAX_BINS/2) + (2 * i)];
      bins[(MAX_BINS/2) - (i + 1)] = bins[(MAX_BINS/2) - ((2 * i) + 1)];
    }
    // finally, zero out the newly-unused bins
    for (size_t i=((MAX_BINS * 3) / 4); i<MAX_BINS; i++) {
      bins[i] = 0;
    }
    for (size_t i=0; i<(MAX_BINS/4); i++) {
      bins[i] = 0;
    }

    // bump up scale by 2x
    double range = scale_max - scale_min;
    scale_max += (range / 2.0);
    scale_min -= (range / 2.0);
  }
}

flexible_type continuous_result::get_min_value() const {
  return this->min;
}

flexible_type continuous_result::get_max_value() const {
  return this->max;
}

continuous_bins continuous_result::get_bins(flex_int num_bins) const {
  if (num_bins < 1) {
    log_and_throw("num_bins must be positive.");
  }
  continuous_bins ret;

  // determine the bin range that covers min to max
  size_t first_bin = get_bin_idx(min, scale_min, scale_max);
  size_t last_bin = get_bin_idx(max, scale_min, scale_max);
  size_t effective_bins = (last_bin - first_bin) + 1;

  // Might end up with fewer effective bins due to very small
  // number of unique values. For now, comment out this assert.
  // TODO -- what should we assert here instead, to make sure we have enough
  // effective range for the desired number of bins? Or should we force
  // discrete histogram for very low cardinality? (At which point, we keep
  // this assertion).
  //DASSERT_GE(effective_bins, (MAX_BINS/4));

  if (num_bins > (MAX_BINS/4)) {
    log_and_throw("num_bins must be less than or equal to the effective number of bins available.");
  }

  // rescale to desired bins, taking more than the effective range if
  // necessary in order to get to num_bins total without resampling
  size_t bins_per_bin = effective_bins / num_bins;
  size_t overflow = effective_bins % num_bins;
  size_t before = 0;
  size_t after = 0;
  if (overflow) {
    overflow = num_bins - overflow;
    bins_per_bin = (effective_bins + overflow) / num_bins;
    before = overflow / 2;
    after = (overflow / 2) + (overflow % 2);
  }

  ret.bins = flex_list(num_bins, 0); // initialize empty
  ret.min = get_value_at_bin(std::max<ssize_t>(0, first_bin - before), scale_min, scale_max, MAX_BINS);
  ret.max = get_value_at_bin(std::min<ssize_t>(last_bin + after + 1, MAX_BINS), scale_min, scale_max, MAX_BINS);
  for (size_t i=0; i<num_bins; i++) {
    for (size_t j=0; j<bins_per_bin; j++) {
      ssize_t idx = (i * bins_per_bin) + j + (first_bin - before);
      if (idx < 0 || idx >= MAX_BINS) {
        // don't try to get values below 0, or past MAX_BINS, that would be silly
        continue;
      }
      ret.bins[i] += this->bins[idx];
    }
  }
  return ret;
}

void continuous_result::update(const flexible_type& value) {
  // ignore undefined values
  if (value.get_type() == flex_type_enum::UNDEFINED) {
    return;
  }

  // assign min/max to return value
  if (value < this->min) { this->min = value; }
  if (value > this->max) { this->max = value; }

  // resize bins if needed
  this->rescale(this->min, this->max);

  // update count in bin
  size_t bin = get_bin_idx(value, this->scale_min, this->scale_max);
  this->bins[bin] += 1;
}

void continuous::init(const gl_sarray& source) {
  continuous_parent::init(source);
  flex_type_enum dtype = m_source.dtype();
  if (dtype != flex_type_enum::INTEGER &&
      dtype != flex_type_enum::FLOAT) {
    log_and_throw("dtype of the provided SArray is not valid for histogram. Only int and float are valid dtypes.");
  }

  size_t input_size = m_source.size();
  if (input_size >= 2 &&
      m_source[0].get_type() != flex_type_enum::UNDEFINED &&
      m_source[1].get_type() != flex_type_enum::UNDEFINED) {
    // start with a sane range for the bins (somewhere near the data)
    // (it can be exceptionally small, since the doubling used in resize()
    // will make it converge to the real range quickly)
    m_transformer.init(m_source[0], m_source[1]);
  } else if (input_size == 1 &&
             m_source[0].get_type() != flex_type_enum::UNDEFINED) {
    // one value, not so interesting
    m_transformer.init(m_source[0], m_source[0]);
  } else {
    // no data
    m_transformer.init(0.0, 0.0);
  }
}

std::vector<continuous_result> continuous::split_input(size_t num_threads) {
  flexible_type current_min = m_transformer.min;
  flexible_type current_max = m_transformer.max;
  double current_scale_min = m_transformer.scale_min;
  double current_scale_max = m_transformer.scale_max;
  std::vector<continuous_result> thread_results(num_threads);
  for (auto& thread_result : thread_results) {
    thread_result.init(current_min, current_max, current_scale_min, current_scale_max);
  }
  return thread_results;
}

void continuous::merge_results(std::vector<continuous_result>& thread_results) {
  for (auto& thread_result : thread_results) {
    flexible_type combined_min = std::min(m_transformer.min, thread_result.min);
    flexible_type combined_max = std::max(m_transformer.max, thread_result.max);
    m_transformer.min = combined_min;
    m_transformer.max = combined_max;
    m_transformer.rescale(combined_min, combined_max);
    thread_result.rescale(combined_min, combined_max);
    DASSERT_EQ(m_transformer.scale_min, thread_result.scale_min);
    DASSERT_EQ(m_transformer.scale_max, thread_result.scale_max);
    for (size_t i=0; i<continuous_result::MAX_BINS; i++) {
      m_transformer.bins[i] += thread_result.bins[i];
    }
  }
}

continuous_result continuous::get_current() { return m_transformer; }


}}}}
