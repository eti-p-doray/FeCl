/*******************************************************************************
 *  \file mxArrayToTrellis.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of toTrellis conversion function.
 ******************************************************************************/

#ifndef TO_TRELLIS_H
#define TO_TRELLIS_H

#include <vector>
#include <memory>
#include <math.h>

#include <mex.h>

#include "CodeStructure/TrellisStructure.h"
#include "MexConversion.h"

using namespace fec;

const char* const trellisProperties[] = {"nextStates", "outputs", "numInputSymbols", "numOutputSymbols", "numStates"};


template <>
class MexConverter<TrellisStructure> {
public:
  static TrellisStructure convert(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    return TrellisStructure(toVector<int>(mxGetField(in, 0, trellisProperties[0])),
                            toVector<int>(mxGetField(in, 0, trellisProperties[1])),
                            log2(toScalar<uint8_t>(mxGetField(in, 0, trellisProperties[2]))),
                            log2(toScalar<uint8_t>(mxGetField(in, 0, trellisProperties[3]))),
                            log2(toScalar<uint8_t>(mxGetField(in, 0, trellisProperties[4]))));
  }
};


#endif