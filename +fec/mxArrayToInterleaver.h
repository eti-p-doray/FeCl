/*******************************************************************************
 *  \file mxArrayToTrellis.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of Interleaver conversion function.
 ******************************************************************************/

#ifndef TO_INTERLEAVER_H
#define TO_INTERLEAVER_H

#include <memory>
#include <math.h>
#include <boost/container/vector.hpp>
#include <mex.h>

#include "MexConversion.h"
#include "CodeStructure/Interleaver.h"

using namespace fec;

template <>
class mxArrayTo<Interleaver> {
public:
  static Interleaver f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    boost::container::vector<size_t> perm = mxArrayTo<boost::container::vector<size_t>>::f(in);
    for (auto & i : perm) {
      i--;
    }
    return Interleaver(perm);
  }
};

#endif
