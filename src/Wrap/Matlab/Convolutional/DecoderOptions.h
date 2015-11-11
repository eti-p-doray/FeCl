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

#ifndef WRAP_CONVOLUTIONAL_DECODER_OPTIONS
#define WRAP_CONVOLUTIONAL_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Convolutional.h"
#include "../util/Conversion.h"

template <>
class mxArrayTo<fec::Convolutional::DecoderOptions> {
public:
  static fec::Convolutional::DecoderOptions f(const mxArray* in) {
    try {
      fec::Convolutional::DecoderOptions decoderOptions;
      decoderOptions.algorithm(  mxArrayTo<fec::DecoderAlgorithm>::f(mxGetField(in, 0, "algorithm")) );
      decoderOptions.scalingFactor(  mxArrayTo<fec::LlrType>::f(mxGetField(in, 0, "scalingFactor")) );
      return decoderOptions;
    } catch (std::exception& e) {
      throw std::invalid_argument("In decoder options: " + std::string(e.what()));
    }
  }
};


inline mxArray* toMxArray(fec::Convolutional::DecoderOptions decoder)
{
  const char* fieldnames[] = {"algorithm", "scalingFactor"};
  mxArray* out = mxCreateStructMatrix(1,1,2, fieldnames);
  
  mxSetField(out, 0, fieldnames[0], toMxArray(decoder.algorithm_));
  mxSetField(out, 0, fieldnames[1], toMxArray(decoder.scalingFactor_));
    
  return out;
}

#endif
