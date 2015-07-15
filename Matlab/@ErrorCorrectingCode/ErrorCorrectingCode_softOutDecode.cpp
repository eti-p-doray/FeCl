/*******************************************************************************
 *  \file ErrorCorrectinCode_softOutDecode.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-07-15
 *
 *  Definition of ErrorCorrectinCode_softOutDecode mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "ErrorCorrectingCode/ErrorCorrectingCode.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 2;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the softOutDecode method in the ErrorCorrectinCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] A-posteriori msg L-values
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  ErrorCorrectinCode object
 *  \param  prhs[1] [in]  parity L-values
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  mexPrintf("1");
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ErrorCorrectingCode_softOutDecode");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ErrorCorrectingCode_softOutDecode");
  }
  
  mexPrintf("2");
  std::unique_ptr<ErrorCorrectingCode> code;
  try {
    mexPrintf("3");
    code = toObject<ErrorCorrectingCode>(prhs[0], "ErrorCorrectingCode");
    mexPrintf("4");
    std::vector<LlrType, MexAllocator<LlrType>> decodedMsg;
    mexPrintf("5");
    code->softOutDecode(toMexVector<LlrType>(prhs[1]), decodedMsg);
    mexPrintf("6");
    plhs[0] = toMxArray(decodedMsg);
    mexPrintf("7");
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}