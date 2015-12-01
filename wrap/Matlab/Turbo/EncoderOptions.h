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

#ifndef WRAP_TURBO_ENCODER_OPTIONS
#define WRAP_TURBO_ENCODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Turbo.h"
#include "../util/Trellis.h"
#include "../util/Permutation.h"
#include "../util/Conversion.h"

template <>
class mxArrayTo<fec::Turbo::EncoderOptions> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Turbo::EncoderOptions operator() (const mxArray* in) const {
    auto trellis = mxArrayTo<std::vector<fec::Trellis>>{msg()}("trellis")(mxGetField(in, 0, "trellis"));
    auto interl = mxArrayTo<std::vector<fec::Permutation>>{msg()}("interleaver")(mxGetField(in, 0, "interleaver"));
    fec::Turbo::EncoderOptions encoderOptions(trellis, interl);
    
    encoderOptions.termination(mxArrayTo<std::vector<fec::Trellis::Termination>>{msg()}("termination")(mxGetField(in, 0, "termination")));
    
    return encoderOptions;
  }
};

#endif
