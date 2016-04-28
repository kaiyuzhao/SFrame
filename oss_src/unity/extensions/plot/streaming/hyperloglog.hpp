#include <unity/lib/toolkit_function_macros.hpp>
#include <unity/lib/toolkit_class_macros.hpp>
#include <sketches/hyperloglog.hpp>
#include <unity/lib/gl_sarray.hpp>

namespace graphlab {
namespace plot {
namespace streaming {

class hyperloglog : public toolkit_class_base {
  private:
    gl_sarray m_source;
    size_t m_currentIdx = 0;
    double m_estimate;
    sketches::hyperloglog m_hll;

  public:
    hyperloglog();
    void init(const gl_sarray& source);
    bool eof() const;
    double error_bound();
    double get();

    BEGIN_CLASS_MEMBER_REGISTRATION("plot.streaming.hyperloglog")
    REGISTER_CLASS_MEMBER_FUNCTION(hyperloglog::eof)
    REGISTER_CLASS_MEMBER_FUNCTION(hyperloglog::get)
    REGISTER_CLASS_MEMBER_FUNCTION(hyperloglog::error_bound)
    REGISTER_CLASS_MEMBER_FUNCTION(hyperloglog::init, "source")
    END_CLASS_MEMBER_REGISTRATION
}; // hyperloglog

}}}
