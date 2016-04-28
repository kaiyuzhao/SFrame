#include <unity/lib/toolkit_function_macros.hpp>
#include <unity/lib/toolkit_class_macros.hpp>

namespace graphlab {
namespace plot {
namespace streaming {

class extrema : public toolkit_class_base {
  private:
    bool m_initialized = false;
    flexible_type m_max;
    flexible_type m_min;

  public:
    bool update(const extrema& value);
    bool update(const flexible_type& value);
    flexible_type get_max() const;
    flexible_type get_min() const;

    BEGIN_CLASS_MEMBER_REGISTRATION("plot.streaming.extrema.1d")
    REGISTER_GETTER("max", extrema::get_max)
    REGISTER_GETTER("min", extrema::get_min)
    END_CLASS_MEMBER_REGISTRATION
};

struct bounding_box : public toolkit_class_base {
  extrema x;
  extrema y;
  bool update(const bounding_box& value);

  BEGIN_CLASS_MEMBER_REGISTRATION("plot.streaming.extrema.2d")
  REGISTER_PROPERTY(x)
  REGISTER_PROPERTY(y)
  END_CLASS_MEMBER_REGISTRATION
};

}}}
