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
 
 Definition of ConvolutionalCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "ConvolutionalCode/ConvolutionalCode.h"
#include "Code.h"
#include "../MexConversion.h"
#include "../mxArrayToTrellis.h"

const int inputCount = 5;
const int outputCount = 1;

const int MapTypeCount = 2;
const char* const MapTypeEnumeration[MapTypeCount] = {
  "LogMap",
  "MaxLogMap",
};

const int TerminationTypeCount = 2;
const char* const TerminationTypeEnumeration[TerminationTypeCount] = {
  "Tail",
  "Truncation"
};

/**
 *  This is the implementation of the constructor in the ConvolutionalCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  trellis state table
 *  \param  prhs[1] [in]  blocSize
 *  \param  prhs[2] [in]  trellis termination type
 *  \param  prhs[3] [in]  decoder algorithm type
 *  \param  prhs[4] [in]  work group size - for parralelisation
 */
void ConvolutionalCode_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ConvolutionalCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ConvolutionalCode_destructor");
  }

  fec::Trellis trellis = mxArrayTo<Trellis>::f(prhs[0]);
  fec::ConvolutionalCode::Structure structure(trellis, mxArrayTo<size_t>::f(prhs[1]), mxArrayTo<fec::ConvolutionalCode::TerminationType>::f(prhs[2],TerminationTypeEnumeration, TerminationTypeCount),
                                                mxArrayTo<fec::ConvolutionalCode::DecoderType>::f(prhs[3],MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(structure, mxArrayTo<size_t>::f(prhs[4]));
  
  plhs[0] = toMxArray(std::move(code));
}