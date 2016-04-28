#include <unity/lib/gl_sarray.hpp>
#include <unity/lib/toolkit_function_macros.hpp>
#include <unity/lib/toolkit_class_macros.hpp>

#include "../transformation.hpp"

namespace graphlab {
namespace _canvas {
namespace streaming {
namespace histogram {

/*
 * continuous_bins()
 * Represents bin values (typically rescaled from original bin contents)
 * along with an effective range (min of first bin, last of max)
 */
struct continuous_bins : public toolkit_class_base {
  flex_list bins;
  double max;
  double min;
  BEGIN_CLASS_MEMBER_REGISTRATION("_canvas.streaming.histogram.continuous.bins")
  REGISTER_PROPERTY(bins)
  REGISTER_PROPERTY(max)
  REGISTER_PROPERTY(min)
  END_CLASS_MEMBER_REGISTRATION
};

/*
 * continuous_result()
 *
 * Stores the intermediate or complete result of a histogram stream.
 *
 * Attributes
 * ----------
 * bins : flex_list<flex_int>
 *     The counts in each bin, in the range scale_min to scale_max.
 * min : flexible_type
 *     The lowest value encountered in the source SArray.
 * max : flexible_type
 *     The highest value encountered in the source SArray.
 * scale_min : double
 *     The low end of the range represented by the bins.
 * scale_max : double
 *     The high end of the range represented by the bins.
 *
 * Methods
 * -------
 * init(flexible_type value1, flexible_type value2)
 *     Initialize the result with the range represented by two values.
 * init(flexible_type value1, flexible_type value2, double scale1, double scale2)
 *     Initialize the result with the range represented by two values and
 *     the scale range (should be >= value range) represented by two scale
 *     values.
 * rescale(double new_min, double new_max)
 *     Rescale the result to the range represented by the provided values.
 */
struct continuous_result : public toolkit_class_base {
  constexpr static size_t MAX_BINS = 1000;
  std::array<flex_int, MAX_BINS> bins;
  flexible_type min;
  flexible_type max;
  double scale_min;
  double scale_max;
  void rescale(double new_min, double new_max);
  void init(flexible_type value1, flexible_type value2);
  void init(flexible_type value1, flexible_type value2, double scale1, double scale2);
  continuous_bins get_bins(flex_int num_bins) const;
  flexible_type get_min_value() const;
  flexible_type get_max_value() const;
  void update(const flexible_type& value); // updates the result w/ value

  BEGIN_CLASS_MEMBER_REGISTRATION("_canvas.streaming.histogram.continuous.result")
  REGISTER_CLASS_MEMBER_FUNCTION(continuous_result::get_bins, "num_bins")
  REGISTER_GETTER("min", continuous_result::get_min_value)
  REGISTER_GETTER("max", continuous_result::get_max_value)
  END_CLASS_MEMBER_REGISTRATION
};

/*
 * continuous()
 *
 * Implements Optimal Streaming Histogram (sort-of) as described in
 * http://blog.amplitude.com/2014/08/06/optimal-streaming-histograms/.
 * dtype of sarray can be flex_int or flex_float
 * continuous histogram always gives bins as flex_ints (bin counts are positive integers).
 *
 * Attributes
 * ----------
 *
 * Methods
 * ----------
 * init(const gl_sarray& source)
 *     Initialize the histogram with an SArray as input.
 * eof()
 *     Returns true if the streaming histogram has covered all input, false
 *     otherwise.
 * get()
 *     Bins (up to) the next BATCH_SIZE values from the input, and returns
 *     the histogram (result type, as shown below) representing the current
 *     distribution of values seen so far.
 */
typedef ::graphlab::_canvas::streaming::transformation<gl_sarray, continuous_result, continuous_result, 1000000> continuous_parent;
class continuous : public continuous_parent {
  public:
    virtual std::vector<continuous_result> split_input(size_t num_threads) override;
    virtual void merge_results(std::vector<continuous_result>& transformers) override;
    virtual continuous_result get_current() override;
    virtual void init(const gl_sarray& source) override;

    BEGIN_CLASS_MEMBER_REGISTRATION("_canvas.streaming.histogram.continuous")
    TRANSFORMATION_REGISTRATION(continuous)
    END_CLASS_MEMBER_REGISTRATION
};

}}}}
