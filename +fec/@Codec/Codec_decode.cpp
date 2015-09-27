/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
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