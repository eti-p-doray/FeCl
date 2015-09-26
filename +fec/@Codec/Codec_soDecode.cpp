/*******************************************************************************
 This file is part of C3sar.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 C3sar is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 C3sar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <mex.h>

#include "Codec.h"
#include "Convolutional/Convolutional.h"
#include "Turbo/Turbo.h"
#include "Ldpc/Ldpc.h"
#include "Structure/Serialization.h"
#include "../util/MexConversion.h"

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
  std::vector<LlrType,Allocator<LlrType>::type> msg;
  std::vector<LlrType,Allocator<LlrType>::type> systOut;
  std::vector<LlrType,Allocator<LlrType>::type> stateOut;
  std::vector<LlrType,Allocator<LlrType>::type> parityOut;
  output.msg(msg);
  if (nlhs > 1) output.syst(systOut);
  if (nlhs > 2) output.state(stateOut);
  if (nlhs > 3) output.parity(parityOut);
  codec->soDecode(input, output);
  
  plhs[0] = toMxArray(msg);
  if (nlhs > 1) plhs[1] = toMxArray(systOut);
  if (nlhs > 2) plhs[2] = toMxArray(stateOut);
  if (nlhs > 3) plhs[3] = toMxArray(parityOut);
}