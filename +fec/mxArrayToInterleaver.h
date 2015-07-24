/*******************************************************************************
 *  \file mxArrayToTrellis.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of toTrellis conversion function.
 ******************************************************************************/

#ifndef TO_INTERLEAVER_H
#define TO_INTERLEAVER_H

#include <vector>
#include <memory>
#include <math.h>

#include <mex.h>

#include "MexConversion.h"
#include "CodeStructure/Interleaver.h"

using namespace fec;

template <>
class MexConverter<Interleaver> {
public:
  static Interleaver convert(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    std::vector<size_t> perm = toVector<size_t>(in);
    for (auto & i : perm) {
      i--;
    }
    return Interleaver(perm);
  }
};

#endif
