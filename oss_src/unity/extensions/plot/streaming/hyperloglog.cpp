/*
* Copyright (C) 2015 Dato, Inc.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "hyperloglog.hpp"

namespace graphlab {
namespace plot {
namespace streaming {

/**
 * An SDK extension that calls the hyperloglog sketch for estimating the number
 * of unique elements in a datastream.
 *
 * Implements the hyperloglog sketch algorithm as described in:
 *   Philippe Flajolet, Eric Fusy, Olivier Gandouet and Frederic Meunier.
 *   HyperLogLog: the analysis of a near-optimal cardinality
 *   estimation algorithm. Conference on Analysis of Algorithms (AofA) 2007.
 *
 * see sketches/hyperloglog.hpp
 *
 * Python usage is simple.
 * \code
 * import graphlab
 * from math import ceil, log
 * hll = graphlab.extensions.plot.streaming.hyperloglog()
 * hll.init(graphlab.SArray( [ceil(log(x)) for x in xrange(1,10000000)] ))
 * while not hll.eof():
 *  print '{0:.0f} unique items'.format(hll.get())
 * print '{0:.2f}% standard error'.format(hll.error_bound())
 * print '{0:.3f} ± {1:.3f} items'.format(hll.get(), hll.get() * hll.error_bound() / 100)
 * \endcode
 */

// default number of buckets
const static size_t BATCH_SIZE = 10000000;
const static size_t BUCKET_SIZE = 16;
const static size_t m_m = 1 << BUCKET_SIZE;

hyperloglog::hyperloglog() : m_hll(BUCKET_SIZE) {}

void hyperloglog::init(const gl_sarray& source) {
  m_source = source;
}

bool hyperloglog::eof() const {
  return m_currentIdx >= m_source.size();
}

/**
 * Returns the standard error.
 *
 * Quoting Flajolet et al.
 * Let sig ~ 1.04 / sqrt(m) represent the standard error; the estimates provided
 * by HYPERLOGLOG are expected to be within sig, 2sig, 3sig of the exact count
 * in respectively 65%, 95%, 99% of all the cases.
 */
double hyperloglog::error_bound() {
  return m_estimate * 1.04 / std::sqrt(m_m);
}

double hyperloglog::get() {

  if (eof()) {
    // return the final cached estimate
    return m_estimate;
  }

  size_t start = m_currentIdx;
  size_t end = std::min(m_currentIdx + BATCH_SIZE, m_source.size());

  for (const auto& value : m_source.range_iterator(start, end)) {
    m_hll.add<flexible_type>(value);
  }
  m_currentIdx = end;

  // cache the estimate calculation
  m_estimate = m_hll.estimate();
  return m_estimate;
}

}}}
