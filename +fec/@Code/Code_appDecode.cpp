/*******************************************************************************
 *  \file ErrorCorrectinCode_appDecode.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-07-15
 *
 *  Definition of ErrorCorrectinCode_appDecode mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "Code.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 3;
const int outputCount = 2;

/*******************************************************************************
 *  This is the implementation of the appDecode method in the ErrorCorrectinCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] msg a posteriori L-values
 *  \param  plhs[1] [out] msg extrinsic L-values
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  ErrorCorrectinCode object
 *  \param  prhs[1] [in]  parity L-values
 *  \param  prhs[2] [in]  msg extrinsic L-values
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in Code_appDecode");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in Code_appDecode");
  }
  
  std::unique_ptr<fec::Code> code;
  try {
    code = toObject<fec::Code>(prhs[0], "Code");
    std::vector<fec::LlrType, MexAllocator<fec::LlrType>> msgOut;
    std::vector<fec::LlrType, MexAllocator<fec::LlrType>> extrinsicOut;
    code->appDecode(toMexVector<fec::LlrType>(prhs[1]), toMexVector<fec::LlrType>(prhs[2]), msgOut, extrinsicOut);
    plhs[0] = toMxArray(msgOut);
    plhs[1] = toMxArray(extrinsicOut);
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}