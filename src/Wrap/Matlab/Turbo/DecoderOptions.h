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

#ifndef WRAP_TURBO_DECODER_OPTIONS
#define WRAP_TURBO_DECODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo.h"
#include "../util/Conversion.h"
#include "../util/Scheduling.h"

template <>
class mxArrayTo<fec::Turbo::DecoderOptions> {
public:
  static fec::Turbo::DecoderOptions f(const mxArray* in) {
    try {
      fec::Turbo::DecoderOptions decoderOptions;
      decoderOptions.iterations(  mxArrayTo<size_t>::f(mxGetField(in, 0, "iterations")) );
      decoderOptions.scheduling(  mxArrayTo<fec::Turbo::Scheduling>::f(mxGetField(in, 0, "scheduling")) );
      decoderOptions.scheduling(  mxArrayTo<fec::Turbo::SchedulingType>::f(mxGetField(in, 0, "schedulingType")) );
      decoderOptions.algorithm(  mxArrayTo<fec::DecoderAlgorithm>::f(mxGetField(in, 0, "algorithm")) );
      decoderOptions.scalingFactor(  mxArrayTo<std::vector<std::vector<double>>>::f(mxGetField(in, 0, "scalingFactor")) );
      
      return decoderOptions;
    } catch (std::exception& e) {
      throw std::invalid_argument("In decoder options: " + std::string(e.what()));
    }
  }
};


inline mxArray* toMxArray(fec::Turbo::DecoderOptions decoder)
{
  const char* fieldnames[] = {"iterations", "schedulingType", "scheduling", "algorithm", "scalingFactor"};
  mxArray* out = mxCreateStructMatrix(1,1, 5, fieldnames);
  
  mxSetField(out, 0, fieldnames[0], toMxArray(decoder.iterations_));
  mxSetField(out, 0, fieldnames[1], toMxArray(decoder.schedulingType_));
  mxSetField(out, 0, fieldnames[1], toMxArray(decoder.scheduling_));
  mxSetField(out, 0, fieldnames[2], toMxArray(decoder.algorithm_));
  mxSetField(out, 0, fieldnames[3], toMxArray(decoder.scalingFactor_));
    
  return out;
}

#endif
