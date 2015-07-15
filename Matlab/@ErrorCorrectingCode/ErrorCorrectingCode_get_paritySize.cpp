/*******************************************************************************
 *  \file ErrorCorrectingCode_get_paritySize.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-14
 *  \version Last update : 2015-07-15
 *
 *  Definition of ErrorCorrectingCode_get_paritySize mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "ErrorCorrectingCode/ErrorCorrectingCode.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 1;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the paritySize getter method
 *  in the ErrorCorrectingCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] paritySize
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  ErrorCorrectingCode object
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ErrorCorrectingCode_get_paritySize");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ErrorCorrectingCode_get_paritySize");
  }
  
  std::unique_ptr<ErrorCorrectingCode> code;
  try {
    code = toObject<ErrorCorrectingCode>(prhs[0], "ErrorCorrectingCode");
    plhs[0] = toMxArray(code->paritySize());
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}