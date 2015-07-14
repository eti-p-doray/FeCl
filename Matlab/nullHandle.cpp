/*******************************************************************************
 *  \file MexConversion.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of general conversion to and from mxArray
 ******************************************************************************/

#include <mex.h>

#include "MexConversion.h"

const int inputCount = 0;
const int outputCount = 1;

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::bad_cast();
  }
  if (nlhs != outputCount) {
    throw std::bad_cast();
  }
  
  std::unique_ptr<int> null;
  plhs[0] = toMxArray(std::move(null));
}