/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Definition of ErrorCorrectingCode_get_msgSize mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "TurboCode/TurboCode.h"
#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 2;
const int outputCount = 0;

/**
 *  This is the implementation of the msgSize getter method
 *  in the ErrorCorrectingCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] msgSize
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  ErrorCorrectingCode object
 */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in Code_get_msgSize");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in Code_get_msgSize");
  }
  
  std::unique_ptr<fec::TurboCode> code;
  try {
    code = mxArrayTo<std::unique_ptr<fec::TurboCode>>::f(prhs[0]);
    code->setIterationCount(mxArrayTo<size_t>::f(prhs[1]));
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}