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

#ifndef MEX_ENCODER_OPTIONS
#define MEX_ENCODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Convolutional/Convolutional.h"
#include "../util/MexTrellis.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Convolutional::EncoderOptions> {
public:
  static fec::Convolutional::EncoderOptions f(const mxArray* in) {
    auto trellis = mxArrayTo<fec::Trellis>::f(mxGetField(in, 0, "trellis"));
    size_t length = mxArrayTo<size_t>::f(mxGetField(in, 0, "length"));
    fec::Convolutional::EncoderOptions encoderOptions(trellis, length);
    
    encoderOptions.termination(mxArrayTo<fec::Convolutional::Termination>::f(mxGetField(in, 0, "termination")));
    
    return encoderOptions;
  }
};

#endif
