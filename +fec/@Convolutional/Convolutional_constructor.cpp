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

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "Convolutional/Convolutional.h"
#include "../util/MexConversion.h"
#include "MexDecoderOptions.h"
#include "MexEncoderOptions.h"

using namespace fec;

const int inputCount = 2;
const int outputCount = 1;

void Convolutional_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount || nlhs != outputCount) {
    throw std::invalid_argument("Wrong arg count");
  }
  Convolutional::EncoderOptions encoderOptions = mxArrayTo<Convolutional::EncoderOptions>::f(prhs[0]);
  Convolutional::DecoderOptions decoderOptions = mxArrayTo<Convolutional::DecoderOptions>::f(prhs[1]);

  Convolutional::Structure structure(encoderOptions, decoderOptions);
  MexHandle<Codec> codec(new Convolutional(structure));
  plhs[0] = toMxArray(std::move(codec));
}