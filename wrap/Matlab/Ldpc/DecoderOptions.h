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

#ifndef WRAP_LDPC_DECODER_OPTIONS
#define WRAP_LDPC_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo.h"
#include "../util/Conversion.h"

template <>
class mxArrayTo<fec::Ldpc::DecoderOptions> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Ldpc::DecoderOptions operator() (const mxArray* in) const {
    fec::Ldpc::DecoderOptions decoderOptions;
    decoderOptions.iterations(  mxArrayTo<size_t>{msg()}("iterations")(mxGetField(in, 0, "iterations")) );
    decoderOptions.algorithm(  mxArrayTo<fec::DecoderAlgorithm>{msg()}("algorithm")(mxGetField(in, 0, "algorithm")) );
    decoderOptions.scalingFactor(  mxArrayTo<std::unordered_map<size_t,std::vector<double>>>{msg()}("scaling factor")(mxGetField(in, 0, "scalingFactor")) );
    
    return decoderOptions;
  }
};


inline mxArray* toMxArray(fec::Ldpc::DecoderOptions decoder)
{
  const char* fieldnames[] = {"iterations", "algorithm", "scalingFactor"};
  mxArray* out = mxCreateStructMatrix(1,1,4, fieldnames);
  
  mxSetField(out, 0, fieldnames[0], toMxArray(decoder.iterations()));
  mxSetField(out, 0, fieldnames[1], toMxArray(decoder.algorithm()));
  mxSetField(out, 0, fieldnames[2], toMxArray(decoder.scalingFactor()));
    
  return out;
}

#endif
