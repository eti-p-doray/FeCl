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

#ifndef WRAP_LDPC_ENCODER_OPTIONS
#define WRAP_LDPC_ENCODER_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Ldpc.h"
#include "../util/BitMatrix.h"
#include "../util/Conversion.h"

template <>
class mxArrayTo<fec::Ldpc::EncoderOptions> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Ldpc::EncoderOptions operator() (const mxArray* in) const {
    auto checkMatrix = mxArrayTo<fec::SparseBitMatrix>{msg()}("check matrix")(mxGetField(in, 0, "checkMatrix"));
    fec::Ldpc::EncoderOptions encoderOptions(checkMatrix);
    return encoderOptions;
  }
};

#endif
