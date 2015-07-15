/*******************************************************************************
 *  \file ConvolutionalCode_softDecode.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Definition of MapCode_decode mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "ErrorCorrectingCode.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 2;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the decode method in the MapCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] parity a posteriori L-values
 *  \param  plhs[1] [out] msg a posteriori L-values
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  MapCode object
 *  \param  prhs[1] [in]  parity extrinsic L-values
 *  \param  prhs[2] [in]  msg extrinsic L-values
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ErrorCorrectingCode_decode");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ErrorCorrectingCode_decode");
  }
  
  std::unique_ptr<ErrorCorrectingCode> code;
  try {
    code = toObject<ErrorCorrectingCode>(prhs[0], "ErrorCorrectingCode");
    std::vector<LlrType, MexAllocator<LlrType>> decodedMsg;
    code->softOutDecode(toMexVector<LlrType>(prhs[1]), decodedMsg);
    plhs[0] = toMxArray(decodedMsg);
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}