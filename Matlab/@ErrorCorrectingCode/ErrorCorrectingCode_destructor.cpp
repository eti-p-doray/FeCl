/*******************************************************************************
 *  \file ConvolutionalCode_destructor.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Definition of ConvolutionalCode_destructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "../MexConversion.h"
#include "../../ErrorCorrectingCode.h"

const int inputCount = 1;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the deleter in the ErrorCorrectingCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] null handle
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  MapCode object to be deleted
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ErrorCorrectingCode_destructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ErrorCorrectingCode_destructor");
  }
  try {
    std::unique_ptr<ErrorCorrectingCode> code = toObject<ErrorCorrectingCode>(prhs[0], "ErrorCorrectingCode");
    code.reset();
    plhs[0] = toMxArray<ErrorCorrectingCode>(std::move(code));
  }
  catch (...) {
    std::unique_ptr<ErrorCorrectingCode> null;
    plhs[0] = toMxArray<ErrorCorrectingCode>(std::move(null));
  }
}