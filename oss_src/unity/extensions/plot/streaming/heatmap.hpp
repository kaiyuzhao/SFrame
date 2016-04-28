#include <unity/lib/gl_sarray.hpp>
#include <unity/lib/toolkit_function_macros.hpp>
#include <unity/lib/toolkit_class_macros.hpp>
#include <memory>

#include "extrema.hpp"

namespace graphlab {
namespace plot {
namespace streaming {

/*
 * heatmap()
 * For now, use a re-binning (restarting) 2-d histogram, ported from Nikhil's
 * Python implementation. TODO: use optimal streaming histogram in 2-d to avoid
 * the need for restarting when re-binning. Until then, this can restart over
 * the same data potentially many times in search of the right extrema.
 *
 * dtype of sarrays can be flex_int or flex_float
 * Heatmap always gives bins as flex_ints (bin counts are positive integers).
 *
 * Attributes
 * ----------
 * BATCH_SIZE : size_t
 *     The number of values to bin in each batch.
 * Methods
 * ----------
 * init(const gl_sarray& source)
 *     Initialize the heatmap with an SArray as input.
 * eof()
 *     Returns true if the streaming heatmap has covered all input, false
 *     otherwise.
 * get()
 *     Bins (up to) the next BATCH_SIZE values from the input, and returns
 *     the heatmap (result type, as shown below) representing the current
 *     distribution of values seen so far.
 */
class heatmap : public toolkit_class_base {
  public:
    struct result : public toolkit_class_base {
      result();
      result(size_t num_bins);
      size_t m_NUM_BINS;
      std::vector<std::vector<flex_int>> bins;
      bounding_box extrema;
      bounding_box bin_extrema;
      flex_int get_num_bins() const;

      BEGIN_CLASS_MEMBER_REGISTRATION("plot.streaming.heatmap.result")
      REGISTER_PROPERTY(bins)
      REGISTER_PROPERTY(extrema)
      REGISTER_PROPERTY(bin_extrema)
      REGISTER_GETTER("num_bins", result::get_num_bins)
      END_CLASS_MEMBER_REGISTRATION
    };

  private:
    size_t m_BATCH_SIZE;
    size_t m_NUM_BINS;
    gl_sarray m_x;
    gl_sarray m_y;
    std::unique_ptr<result> m_result;
    size_t m_currentIdx = 0;
    bool m_initialized = false;
    void double_range(extrema& curr, const extrema& prev);
    size_t get_bin_idx(flexible_type value, flexible_type min, flexible_type max);
  public:
    void init(const gl_sarray& x, const gl_sarray& y, size_t num_bins, size_t batch_size);
    result get();
    bool eof() const;
    flex_int get_rows_processed() const;

    BEGIN_CLASS_MEMBER_REGISTRATION("plot.streaming.heatmap")
    REGISTER_CLASS_MEMBER_FUNCTION(heatmap::eof)
    REGISTER_CLASS_MEMBER_FUNCTION(heatmap::get)
    REGISTER_CLASS_MEMBER_FUNCTION(heatmap::init, "x", "y", "num_bins", "batch_size")
    REGISTER_GETTER("rows_processed", heatmap::get_rows_processed)
    END_CLASS_MEMBER_REGISTRATION
};

}}}
