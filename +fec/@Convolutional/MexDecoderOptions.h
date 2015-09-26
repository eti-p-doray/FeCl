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

#ifndef MEX_DECODER_OPTIONS
#define MEX_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Convolutional/Convolutional.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Convolutional::DecoderOptions> {
public:
  static fec::Convolutional::DecoderOptions f(const mxArray* in) {
    fec::Convolutional::DecoderOptions decoderOptions;
    decoderOptions.algorithm(  mxArrayTo<fec::Codec::DecoderAlgorithm>::f(mxGetField(in, 0, "algorithm")) );
    
    return decoderOptions;
  }
};


inline mxArray* toMxArray(fec::Convolutional::DecoderOptions decoder)
{
  const char* fieldnames[] = {"algorithm"};
  mxArray* out = mxCreateStructMatrix(1,1,1, fieldnames);
  
  mxSetField(out, 0, "algorithm", toMxArray(decoder.algorithm_));
    
  return out;
}

#endif
