#include "heatmap.hpp"
#include <parallel/lambda_omp.hpp>

// constexpr static size_t m_NUM_BINS = 1000;
constexpr static size_t INITIAL_EXTREMA_SAMPLE_SIZE = 10000;

namespace graphlab {
namespace plot {
namespace streaming {

static inline gl_sarray validate_dtype(const gl_sarray& input) {
  flex_type_enum dtype = input.dtype();
  if (dtype != flex_type_enum::INTEGER &&
      dtype != flex_type_enum::FLOAT) {
    log_and_throw("dtype of the provided SArray is not valid for heatmap. heatmap can only operate on INTEGER or FLOAT SArrays.");
  }
  return input;
}

static inline void set_extrema(
  const gl_sarray& input, extrema& extrema,
  bool ensure_range=false /* make sure min and max are different */
) {
  // use the first and last INITIAL_EXTREMA_SAMPLE_SIZE from the dataset,
  // to account for monotonically increasing data
  const size_t sample_size = std::min(INITIAL_EXTREMA_SAMPLE_SIZE, input.size());
  for (const auto& val : input.range_iterator(0, sample_size)) {
    // ignore undefined values
    if (val.get_type() == flex_type_enum::UNDEFINED) {
      continue;
    }
    extrema.update(val);
  }
  for (const auto& val : input.range_iterator(
      input.size() - sample_size,
      input.size())) {
    // ignore undefined values
    if (val.get_type() == flex_type_enum::UNDEFINED) {
      continue;
    }
    extrema.update(val);
  }
  if (ensure_range &&
      extrema.get_min() == extrema.get_max()) {
    if (input.dtype() == flex_type_enum::INTEGER) {
      extrema.update(extrema.get_min() + 1);
    } else if (input.dtype() == flex_type_enum::FLOAT) {
      extrema.update(extrema.get_min() + 1e-12);
    }
  }
}
heatmap::result::result(){}

heatmap::result::result(size_t num_bins) : m_NUM_BINS(num_bins), bins(num_bins, std::vector<flex_int>(num_bins, 0)) {}

flex_int heatmap::result::get_num_bins() const {
  return m_NUM_BINS;
};

void heatmap::init(const gl_sarray& x, const gl_sarray& y, size_t num_bins, size_t batch_size) {
  if (m_initialized) {
    log_and_throw("heatmap already initialized.");
  }
  m_BATCH_SIZE = batch_size;
  m_NUM_BINS = num_bins;
  m_result.reset(new result(num_bins));
  m_initialized = true;
  m_x = validate_dtype(x);
  m_y = validate_dtype(y);

  if (m_x.size() != m_y.size()) {
    log_and_throw("heatmap can only operate on two SArrays of the same length.");
  }

  // probe initial bounds
  set_extrema(m_x, m_result->extrema.x);
  set_extrema(m_x, m_result->bin_extrema.x, true /* ensure_range */);
  set_extrema(m_y, m_result->extrema.y);
  set_extrema(m_y, m_result->bin_extrema.y, true /* ensure_range */);
}

inline size_t heatmap::get_bin_idx(
  flexible_type value,
  flexible_type min,
  flexible_type max
) {
  size_t ret = static_cast<size_t>(
    std::floor(
      (static_cast<double>(value - min) /
       static_cast<double>(max - min)) *
      static_cast<double>(m_NUM_BINS)
    )
  );
  if (ret == m_NUM_BINS) {
    ret--;
  }
  DASSERT_LT(ret, m_NUM_BINS);
  return ret;
}

inline void heatmap::double_range(extrema& curr, const extrema& prev) {
  flexible_type max = prev.get_max();
  flexible_type min = prev.get_min();
  flexible_type range = max - min;
  curr.update(max + (range / 2));
  curr.update(min - (range / 2));
}

heatmap::result heatmap::get() {
  if (this->eof()) {
    return *(m_result.get());
  }

  size_t start = m_currentIdx;
  m_currentIdx = std::min(start + m_BATCH_SIZE, m_x.size());
  size_t input_size = m_currentIdx - start;
  const auto& prev_extrema = m_result->extrema;
  const auto& source_x = m_x;
  const auto& source_y = m_y;

  size_t num_threads_reported = thread_pool::get_instance().size();
  std::vector< std::unique_ptr<heatmap::result> > thread_results(num_threads_reported);
  std::vector<bool> threads_outside_range(num_threads_reported);
  for (auto& thread_result : thread_results) {
    // initialize with current extrema
    thread_result.reset(new result(this->m_NUM_BINS));
    thread_result->extrema = m_result->extrema;
    thread_result->bin_extrema = m_result->bin_extrema;
  }

  in_parallel(
    [this, start, input_size, &thread_results, &threads_outside_range, &source_x, &source_y, num_threads_reported]
    (size_t thread_idx, size_t num_threads) {

    DASSERT_EQ(num_threads, num_threads_reported);
    size_t thread_input_size = input_size / num_threads;
    size_t thread_start = start + (thread_idx * thread_input_size);
    size_t thread_end = thread_idx == num_threads - 1 ?
      start + input_size :
      thread_start + thread_input_size;
    DASSERT_LE(thread_end, start + input_size);
    auto& thread_result = thread_results[thread_idx];

    auto x_range = source_x.range_iterator(thread_start, thread_end);
    auto y_range = source_y.range_iterator(thread_start, thread_end);
    auto x_it = x_range.begin();
    auto y_it = y_range.begin();

    for (; x_it != x_range.end() && y_it != y_range.end(); x_it++, y_it++) {
      const auto& x_val = *x_it;
      const auto& y_val = *y_it;

      // ignore undefined values
      if (x_val.get_type() == flex_type_enum::UNDEFINED ||
          y_val.get_type() == flex_type_enum::UNDEFINED) {
        continue;
      }

      threads_outside_range[thread_idx] = thread_result->bin_extrema.x.update(x_val) || threads_outside_range[thread_idx];
      threads_outside_range[thread_idx] = thread_result->bin_extrema.y.update(y_val) || threads_outside_range[thread_idx];

      if (threads_outside_range[thread_idx]) {
        // don't bother binning more values in this chunk, since they could
        // also be out of range. just find the new extrema and re-start after that
        continue;
      }

      thread_result->extrema.x.update(x_val);
      thread_result->extrema.y.update(y_val);

      size_t x_bin = this->get_bin_idx(x_val, thread_result->bin_extrema.x.get_min(), thread_result->bin_extrema.x.get_max());
      //the output range of get_bin-idx is 0 to m_NUM_BINS-1;
      size_t y_bin = m_NUM_BINS - 1- this->get_bin_idx(y_val, thread_result->bin_extrema.y.get_min(), thread_result->bin_extrema.y.get_max());
      thread_result->bins[x_bin][y_bin]++;
    }
  });

  // first, combine bin_extrema, and keep track of value_outside_range
  bool value_outside_range = false;
  for (bool thread_outside_range : threads_outside_range) {
    value_outside_range = value_outside_range || thread_outside_range;
  }
  for (const auto& thread_result : thread_results) {
    m_result->bin_extrema.update(thread_result->bin_extrema);
  }

  if (value_outside_range) {
    // encountered new extrema
    // restart binning from the beginning, keeping the currently known extrema
    // TODO -- use the optimal streaming histogram technique to avoid needing to restart
    m_currentIdx = 0;
    m_result->bins = std::vector<std::vector<flex_int>>(m_NUM_BINS, std::vector<flex_int>(m_NUM_BINS, 0));

    // double range if necessary
    this->double_range(m_result->bin_extrema.x, prev_extrema.x);
    this->double_range(m_result->bin_extrema.y, prev_extrema.y);

    return this->get();
  }

  // combine extrema and counts
  for (const auto& thread_result : thread_results) {
    m_result->extrema.update(thread_result->extrema);
    for (size_t i=0; i<m_NUM_BINS; i++) {
      for (size_t j=0; j<m_NUM_BINS; j++) {
        m_result->bins[i][j] += thread_result->bins[i][j];
      }
    }
  }

  return *(m_result.get());
}

bool heatmap::eof() const {
  DASSERT_LE(m_currentIdx, m_x.size());
  return m_currentIdx == m_x.size();
}

flex_int heatmap::get_rows_processed() const {
  return m_currentIdx;
}

}}}
