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
 
 Definition of TurboCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "TurboCode/TurboCode.h"
#include "Code.h"

#include "../MexConversion.h"
#include "../mxArrayToTrellis.h"
#include "../mxArrayToInterleaver.h"

const int inputCount = 7;
const int outputCount = 1;

const int TrellisEndTypeCount = 2;
const char* const TrellisEndTypeEnumeration[TrellisEndTypeCount] = {
  "PaddingTail",
  "Truncation"
};

const int StructureTypeCount = 2;
const char* const StructureTypeEnumeration[StructureTypeCount] = {
  "Serial",
  "Parallel",
};

const int MapTypeCount = 2;
const char* const MapTypeEnumeration[MapTypeCount] = {
  "LogMap",
  "MaxLogMap",
};

/**
 *  This is the implementation of the constructor in the ConvolutionalCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  trellis
 *  \param  prhs[1] [in]  Interleaver
 *  \param  prhs[2] [in]  Trellis end type
 *  \param  prhs[3] [in]  Maximum nomber of iterations
 *  \param  prhs[4] [in]  trellis termination type
 *  \param  prhs[5] [in]  decoder algorithm type
 *  \param  prhs[6] [in]  work group size - for parralelisation
 */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in TurboCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in TurboCode_destructor");
  }
  
  std::vector<fec::TrellisStructure> trellis = mxCellArrayTo<fec::TrellisStructure>::f(prhs[0]);
  std::vector<fec::Interleaver> interleavers = mxCellArrayTo<fec::Interleaver>::f(prhs[1]);
  std::vector<fec::ConvolutionalCodeStructure::TrellisEndType> endType =
  mxCellArrayTo<fec::ConvolutionalCodeStructure::TrellisEndType>::f(prhs[2], TrellisEndTypeEnumeration, TrellisEndTypeCount);
  
  fec::TurboCodeStructure codeStructure(trellis, interleavers, endType, mxArrayTo<size_t>::f(prhs[3]),
                                        mxArrayTo<fec::TurboCodeStructure::DecoderType>::f(prhs[4], StructureTypeEnumeration, StructureTypeCount),
                                        mxArrayTo<fec::ConvolutionalCodeStructure::DecoderType>::f(prhs[5],MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure, mxArrayTo<size_t>::f(prhs[6]));
  
  plhs[0] = toMxArray(std::move(code));
}