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

#ifndef MEX_ENCODER_OPTIONS
#define MEX_ENCODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo/Turbo.h"
#include "../util/MexTrellis.h"
#include "../util/MexPermutation.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Turbo::EncoderOptions> {
public:
  static fec::Turbo::EncoderOptions f(const mxArray* in) {
    auto trellis = mxArrayTo<std::vector<fec::Trellis>>::f(mxGetField(in, 0, "trellis"));
    auto interl = mxArrayTo<std::vector<fec::Permutation>>::f(mxGetField(in, 0, "interleaver"));
    fec::Turbo::EncoderOptions encoderOptions(trellis, interl);
    
    encoderOptions.termination(mxArrayTo<std::vector<fec::Convolutional::Termination>>::f(mxGetField(in, 0, "termination")));
    
    return encoderOptions;
  }
};

#endif
