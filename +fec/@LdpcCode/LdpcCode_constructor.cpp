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
 
 Definition of LdpcCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>
#include <vector>
#include <random>

#include <mex.h>

#include "LdpcCode/LdpcCode.h"
#include "../MexConversion.h"
#include "../mxArrayToBitMatrix.h"

const int inputCount = 4;
const int outputCount = 1;

const int BpTypeCount = 3;
const char* const BpTypeEnumeration[BpTypeCount] = {
  "TrueBp",
  "MinSumBp",
  "ScoreBp",
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& a)
{
  for (uint64_t i = 0; i < a.size(); i++) {
    os << float(a[i]) << " ";
  }
  return os;
}

/**
 *  This is the implementation of the constructor in the LdpcCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  parityCheck matrix
 *  \param  prhs[1] [in]  Maximum number of iterations
 *  \param  prhs[2] [in]  Decoder algorithm type
 *  \param  prhs[3] [in]  work group size - for parralelisation
 */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in LdpcCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in LdpcCode_constructor");
  }
  
  fec::LdpcCodeStructure codeStructure(mxArrayTo<SparseBitMatrix>::f(prhs[0]), mxArrayTo<size_t>::f(prhs[1]), mxArrayTo<LdpcCodeStructure::DecoderType>::f(prhs[2],BpTypeEnumeration, BpTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure,mxArrayTo<size_t>::f(prhs[3]));
  
  plhs[0] = toMxArray(std::move(code));
}