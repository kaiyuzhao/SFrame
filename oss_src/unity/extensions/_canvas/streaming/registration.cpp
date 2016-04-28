#include "groupby.hpp"
#include "heatmap.hpp"
#include "histogram/continuous.hpp"
#include "hyperloglog.hpp"
#include "sample.hpp"

using namespace graphlab;

BEGIN_CLASS_REGISTRATION
REGISTER_CLASS(graphlab::_canvas::streaming::bounding_box);
REGISTER_CLASS(graphlab::_canvas::streaming::extrema);
REGISTER_CLASS(graphlab::_canvas::streaming::groupby_quantile);
REGISTER_CLASS(graphlab::_canvas::streaming::groupby_quantile_result);
REGISTER_CLASS(graphlab::_canvas::streaming::groupby_summary);
REGISTER_CLASS(graphlab::_canvas::streaming::groupby_summary_result);
REGISTER_CLASS(graphlab::_canvas::streaming::heatmap);
REGISTER_CLASS(graphlab::_canvas::streaming::heatmap::result);
REGISTER_CLASS(graphlab::_canvas::streaming::histogram::continuous);
REGISTER_CLASS(graphlab::_canvas::streaming::histogram::continuous_bins);
REGISTER_CLASS(graphlab::_canvas::streaming::histogram::continuous_result);
REGISTER_CLASS(graphlab::_canvas::streaming::hyperloglog);
REGISTER_CLASS(graphlab::_canvas::streaming::sarray_sample);
REGISTER_CLASS(graphlab::_canvas::streaming::sframe_sample);
END_CLASS_REGISTRATION
