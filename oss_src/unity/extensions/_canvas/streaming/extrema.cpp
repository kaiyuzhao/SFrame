#include "extrema.hpp"

namespace graphlab {
namespace _canvas {
namespace streaming {

flexible_type extrema::get_max() const {
  return m_max;
}

flexible_type extrema::get_min() const {
  return m_min;
}

bool extrema::update(const flexible_type& value) {
  if (!m_initialized) {
    m_initialized = true;
    m_max = value;
    m_min = value;
    return true;
  }

  if (value > m_max) {
    m_max = value;
    return true;
  }

  if (value < m_min) {
    m_min = value;
    return true;
  }

  return false;
}

bool extrema::update(const extrema& value) {
  return this->update(value.get_min()) || this->update(value.get_max());
}

bool bounding_box::update(const bounding_box& value) {
  return this->x.update(value.x) || this->y.update(value.y);
}

}}}
