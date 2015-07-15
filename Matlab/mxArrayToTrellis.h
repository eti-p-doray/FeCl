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

TrellisStructure toTrellisStructure(const mxArray* nextState, const mxArray* output, const mxArray* inputCount, const mxArray* outputCount, const mxArray* stateCount)
{
  return TrellisStructure(toVector<int>(nextState), toVector<int>(output), log2(toScalar<uint8_t>(inputCount)), log2(toScalar<uint8_t>(outputCount)), log2(toScalar<uint8_t>(stateCount)));
}

#endif
