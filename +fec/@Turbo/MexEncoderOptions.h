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

#ifndef MEX_ENCODER_OPTIONS
#define MEX_ENCODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo/Turbo.h"
#include "../util/MexTrellis.h"
#include "../util/MexInterleaver.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Turbo::EncoderOptions> {
public:
  static fec::Turbo::EncoderOptions f(const mxArray* in) {
    auto trellis = mxArrayTo<std::vector<fec::Trellis>>::f(mxGetField(in, 0, "trellis"));
    auto interl = mxArrayTo<std::vector<fec::Interleaver>>::f(mxGetField(in, 0, "interleaver"));
    fec::Turbo::EncoderOptions encoderOptions(trellis, interl);
    
    encoderOptions.termination(mxArrayTo<std::vector<fec::Convolutional::Termination>>::f(mxGetField(in, 0, "termination")));
    encoderOptions.bitOrdering(mxArrayTo<fec::Turbo::BitOrdering>::f(mxGetField(in, 0, "bitOrdering")));
    
    return encoderOptions;
  }
};

#endif
