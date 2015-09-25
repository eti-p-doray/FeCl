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
 ******************************************************************************/

#include <mex.h>

#include "Codec.h"
#include "Convolutional/Convolutional.h"
#include "Turbo/Turbo.h"
#include "Ldpc/Ldpc.h"
#include "Structure/Serialization.h"
#include "../util/MexConversion.h"
#include "../util/MexBitField.h"

using namespace fec;

const int inputCount = 2;
const int outputCount = 1;

void Codec_decode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  DerivedTypeHolder<Convolutional,Turbo,Ldpc> derived;
  auto codec = mxArrayTo<MexHandle<Codec>>::f(prhs[0], derived);
  
  std::vector<LlrType,Allocator<LlrType>::type> parity;
  try {
    parity = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[1]);
  } catch (std::exception& e) {
    throw std::invalid_argument("Parity vector is invalid");
  }
  std::vector<BitField<bool>, Allocator<BitField<bool>>::type> msg;
  codec->decode(parity, msg);
  plhs[0] = toMxArray(msg);
}