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
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
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

void Codec_encode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  DerivedTypeHolder<Convolutional,Turbo,Ldpc> derived;
  auto codec = mxArrayTo<MexHandle<Codec>>::f(prhs[0], derived);
  
  std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type> msg;
  try {
    msg = mxArrayTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type>>::f(prhs[1]);
  } catch (std::exception& e) {
    throw std::invalid_argument("Msg vector is invalid");
  }
  std::vector<BitField<size_t>, Allocator<BitField<size_t>>::type> parity;
  codec->encode(msg, parity);
  plhs[0] = toMxArray(parity);
}