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

#ifndef CONVOLUTIONAL_MEX_PUNCTURE_OPTIONS
#define CONVOLUTIONAL_MEX_PUNCTURE_OPTIONS

#include <memory>
#include <type_traits>

#include <mex.h>

#include "Convolutional/Convolutional.h"
#include "../util/MexPermutation.h"
#include "../util/MexConversion.h"

template <>
class mxArrayTo<fec::Convolutional::PunctureOptions> {
public:
  static fec::Convolutional::PunctureOptions f(const mxArray* in) {
    fec::Convolutional::PunctureOptions punctureOptions(mxArrayTo<std::vector<bool>>::f(mxGetField(in, 0, "mask")));
    
    punctureOptions.tailMask(mxArrayTo<std::vector<bool>>::f(mxGetField(in, 0, "tailMask")));
    
    return punctureOptions;
  }
};

#endif
