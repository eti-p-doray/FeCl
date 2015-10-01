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

#ifndef MEX_DECODER_OPTIONS
#define MEX_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo/Turbo.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Ldpc::DecoderOptions> {
public:
  static fec::Ldpc::DecoderOptions f(const mxArray* in) {
    fec::Ldpc::DecoderOptions decoderOptions;
    try {
      decoderOptions.iterations(  mxArrayTo<size_t>::f(mxGetField(in, 0, "iterations")) );
    } catch (...) {
      throw std::invalid_argument("Invalid iterations");
    }
    try {
      decoderOptions.algorithm(  mxArrayTo<fec::Codec::DecoderAlgorithm>::f(mxGetField(in, 0, "algorithm")) );
    } catch (...) {
      throw std::invalid_argument("Invalid algorithm");
    }
    
    return decoderOptions;
  }
};


inline mxArray* toMxArray(fec::Ldpc::DecoderOptions decoder)
{
  const char* fieldnames[] = {"iterations", "algorithm"};
  mxArray* out = mxCreateStructMatrix(1,1,2, fieldnames);
  
  mxSetField(out, 0, "iterations", toMxArray(decoder.iterations_));
  mxSetField(out, 0, "algorithm", toMxArray(decoder.algorithm_));
    
  return out;
}

#endif
