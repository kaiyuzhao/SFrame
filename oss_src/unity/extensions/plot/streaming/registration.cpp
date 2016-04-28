#include "groupby.hpp"
#include "heatmap.hpp"
#include "histogram/continuous.hpp"
#include "hyperloglog.hpp"
#include "sample.hpp"

using namespace graphlab;

BEGIN_CLASS_REGISTRATION
REGISTER_CLASS(graphlab::plot::streaming::bounding_box);
REGISTER_CLASS(graphlab::plot::streaming::extrema);
REGISTER_CLASS(graphlab::plot::streaming::groupby_quantile);
REGISTER_CLASS(graphlab::plot::streaming::groupby_quantile_result);
REGISTER_CLASS(graphlab::plot::streaming::groupby_summary);
REGISTER_CLASS(graphlab::plot::streaming::groupby_summary_result);
REGISTER_CLASS(graphlab::plot::streaming::heatmap);
REGISTER_CLASS(graphlab::plot::streaming::heatmap::result);
REGISTER_CLASS(graphlab::plot::streaming::histogram::continuous);
REGISTER_CLASS(graphlab::plot::streaming::histogram::continuous_bins);
REGISTER_CLASS(graphlab::plot::streaming::histogram::continuous_result);
REGISTER_CLASS(graphlab::plot::streaming::hyperloglog);
REGISTER_CLASS(graphlab::plot::streaming::sarray_sample);
REGISTER_CLASS(graphlab::plot::streaming::sframe_sample);
END_CLASS_REGISTRATION
