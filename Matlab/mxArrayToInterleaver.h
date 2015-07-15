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

#include "CodeStructure/Interleaver.h"

Interleaver toInterleaver(const mxArray* in)
{
  std::vector<size_t> perm = toVector<size_t>(in);
  for (auto & i : perm) {
    i--;
  }
  return Interleaver(perm);
}

#endif
