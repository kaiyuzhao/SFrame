#include <random/random.hpp>
#include <unity/lib/toolkit_function_macros.hpp>
#include <unity/lib/toolkit_class_macros.hpp>
#include <unity/lib/gl_sarray.hpp>
#include <unity/lib/gl_sframe.hpp>

#include <set>

namespace graphlab {
namespace _canvas {
namespace streaming {


/*
 * reservoir sample, on SFrame or SArray, as described at
 * https://en.wikipedia.org/wiki/Reservoir_sampling
 */

template<typename T>
class sample : public toolkit_class_base {
  public:
    typedef std::vector<flexible_type> result;

  private:
    constexpr static size_t SAMPLE_SIZE = 1000;
    constexpr static size_t BATCH_SIZE = 1000000;
    T m_source;
    size_t m_currentIdx;
    result m_result;

  public:
    sample() {}
    void init(const T& source) {
      m_currentIdx = 0;
      m_result = result();
      m_source = source;
    }
    bool eof() const {
      return m_currentIdx >= m_source.size();
    }
    flex_int get_rows_processed() const {
      return m_currentIdx;
    }
    result get() {
      if (eof()) {
        return m_result;
      }

      // initial fill
      if (m_currentIdx == 0) {
        for (const auto& value : m_source.head(SAMPLE_SIZE).range_iterator()) {
          m_currentIdx++;
          m_result.push_back(value);
        }
      }

      // reservoir sample
      if (m_currentIdx < m_source.size()) {
        graphlab::random::generator gen;
        const size_t start = m_currentIdx;
        const size_t end = std::min(m_currentIdx + BATCH_SIZE, m_source.size());
        std::set<size_t> indices;

        // gather indices to sample from
        for (size_t i=start; i<end; i++) {
          const size_t j = gen.fast_uniform<size_t>(0, i);
          if (j < SAMPLE_SIZE) {
            indices.insert(i);
          }
        }

        // seek to each index and get value, insert into sample
        for (size_t index_in_sframe : indices) {
          const auto& value = m_source[index_in_sframe];

          size_t index_in_sample = gen.fast_uniform<size_t>(0, SAMPLE_SIZE-1);
          m_result[index_in_sample] = value;

        }

        m_currentIdx = end;
      }

      return m_result;
    }
}; // sample

class sarray_sample : public sample<gl_sarray> {
  public:
    BEGIN_CLASS_MEMBER_REGISTRATION("_canvas.streaming.sarray_sample")
    REGISTER_CLASS_MEMBER_FUNCTION(sarray_sample::eof)
    REGISTER_CLASS_MEMBER_FUNCTION(sarray_sample::get)
    REGISTER_CLASS_MEMBER_FUNCTION(sarray_sample::init, "source")
    REGISTER_GETTER("rows_processed", sarray_sample::get_rows_processed)
    END_CLASS_MEMBER_REGISTRATION
}; // sarray_sample

class sframe_sample : public sample<gl_sframe> {
  public:
    BEGIN_CLASS_MEMBER_REGISTRATION("_canvas.streaming.sframe_sample")
    REGISTER_CLASS_MEMBER_FUNCTION(sframe_sample::eof)
    REGISTER_CLASS_MEMBER_FUNCTION(sframe_sample::get)
    REGISTER_CLASS_MEMBER_FUNCTION(sframe_sample::init, "source")
    REGISTER_GETTER("rows_processed", sframe_sample::get_rows_processed)
    END_CLASS_MEMBER_REGISTRATION
}; // sframe_sample

}}}
