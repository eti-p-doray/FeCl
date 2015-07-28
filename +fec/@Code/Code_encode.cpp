/*******************************************************************************
 *  \file ErrorCorrectingCode_encode.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-07-15
 *
 *  Declaration of ErrorCorrectingCode_encode mex function.
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "Code.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 2;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the encode method in the ConvolutionalCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] parity bits array
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  ErrorCorrectingCode object
 *  \param  prhs[1] [in]  msg bits array
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in Code_encode");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in Code_encode");
  }
  
  std::unique_ptr<fec::Code> code;
  try {
    code = mxArrayTo<std::unique_ptr<fec::Code>>::f(prhs[0]);
    std::vector<uint8_t, MexAllocator<uint8_t>> parity;
    
    code->encode(mxArrayTo<std::vector<uint8_t,MexAllocator<uint8_t>>>::f(prhs[1]), parity);
  
    plhs[0] = toMxArray(parity);
  } 
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  code.release();
}