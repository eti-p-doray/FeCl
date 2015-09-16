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
#include "../MexConversion.h"

using namespace fec;

void Codec_soDecode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{  
  DerivedTypeHolder<Convolutional,Turbo,Ldpc> derived;
  auto codec = mxArrayTo<MexHandle<Codec>>::f(prhs[0], derived);
  
  std::vector<LlrType,Allocator<LlrType>::type> parityIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[1]);
  std::vector<LlrType,Allocator<LlrType>::type> stateIn;
  if (nrhs > 2) stateIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[2]);
  std::vector<LlrType,Allocator<LlrType>::type> systIn;
  if (nrhs > 3) systIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[3]);
  
  auto input = Codec::Info<const std::vector<LlrType,Allocator<LlrType>::type>>();
  if (parityIn.size()) input.parity(parityIn);
  if (stateIn.size()) input.state(stateIn);
  if (systIn.size()) input.syst(systIn);

  auto output = Codec::Info<std::vector<LlrType,Allocator<LlrType>::type>>();
  std::vector<LlrType,Allocator<LlrType>::type> systOut;
  std::vector<LlrType,Allocator<LlrType>::type> stateOut;
  std::vector<LlrType,Allocator<LlrType>::type> parityOut;
  if (nlhs > 0) output.syst(systOut);
  if (nlhs > 1) output.state(stateOut);
  if (nlhs > 2) output.parity(parityOut);
  codec->soDecode(input, output);
  
  if (nlhs > 0) plhs[0] = toMxArray(systOut);
  if (nlhs > 1) plhs[1] = toMxArray(stateOut);
  if (nlhs > 2) plhs[2] = toMxArray(parityOut);
}