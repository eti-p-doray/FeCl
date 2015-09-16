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
 
 Definition of the mex allocator.
 ******************************************************************************/

#ifndef MEX_DECODER_OPTIONS
#define MEX_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo/Turbo.h"
#include "../MexConversion.h"

template <>
class mxArrayTo<fec::Turbo::DecoderOptions> {
public:
  static fec::Turbo::DecoderOptions f(const mxArray* in) {
    fec::Turbo::DecoderOptions decoderOptions;
    decoderOptions.iterations(  mxArrayTo<size_t>::f(mxGetField(in, 0, "iterations")) );
    decoderOptions.scheduling(  mxArrayTo<fec::Turbo::Scheduling>::f(mxGetField(in, 0, "scheduling")) );
    decoderOptions.algorithm(  mxArrayTo<fec::Codec::DecoderAlgorithm>::f(mxGetField(in, 0, "algorithm")) );
    
    return decoderOptions;
  }
};


inline mxArray* toMxArray(fec::Turbo::DecoderOptions decoder)
{
  const char* fieldnames[] = {"iterations", "scheduling", "algorithm"};
  mxArray* out = mxCreateStructMatrix(1,1, 3, fieldnames);
  
  mxSetField(out, 0, "iterations", toMxArray(decoder.iterations_));
  mxSetField(out, 0, "scheduling", toMxArray(decoder.scheduling_));
  mxSetField(out, 0, "algorithm", toMxArray(decoder.algorithm_));
    
  return out;
}

#endif
