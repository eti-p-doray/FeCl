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

#ifndef WRAP_CONVOLUTIONAL_PUNCTURE_OPTIONS
#define WRAP_CONVOLUTIONAL_PUNCTURE_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Convolutional.h"
#include "../util/Permutation.h"
#include "../util/Conversion.h"

template <>
class mxArrayTo<fec::Convolutional::PunctureOptions> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Convolutional::PunctureOptions operator() (const mxArray* in) const {
    fec::Convolutional::PunctureOptions punctureOptions;
    punctureOptions.mask(mxArrayTo<std::vector<bool>>{msg()}("mask")(mxGetField(in, 0, "mask")));
    punctureOptions.tailMask(mxArrayTo<std::vector<bool>>{msg()}("tailMask")(mxGetField(in, 0, "tailMask")));
    
    return punctureOptions;
  }
};

#endif
