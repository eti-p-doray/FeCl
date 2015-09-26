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

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "Ldpc/Ldpc.h"
#include "../util/MexConversion.h"
#include "MexDecoderOptions.h"
#include "MexEncoderOptions.h"

using namespace fec;

const int inputCount = 2;
const int outputCount = 1;

void Ldpc_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  Ldpc::EncoderOptions encoderOptions = mxArrayTo<Ldpc::EncoderOptions>::f(prhs[0]);
  Ldpc::DecoderOptions decoderOptions = mxArrayTo<Ldpc::DecoderOptions>::f(prhs[1]);

  Ldpc::Structure structure(encoderOptions, decoderOptions);
  MexHandle<Codec> codec(new Ldpc(structure));
  plhs[0] = toMxArray(std::move(codec));
}