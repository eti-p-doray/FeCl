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

#ifndef WRAP_SCHEDULING_H
#define WRAP_SCHEDULING_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>

#include "Turbo.h"
#include "Conversion.h"
#include "BitField.h"

template<>
class mxArrayTo<fec::Turbo::Scheduling> {
public:
  static fec::Turbo::Scheduling f(const mxArray* in) {
    if (in == nullptr) {
      return {};
    }
    
    fec::Turbo::Scheduling out;
    out.stages.resize(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.stages.size(); ++i) {
      out.stages[i] = {mxArrayTo<std::vector<size_t>>::f(mxGetField(in, i, "activation")), mxArrayTo<std::vector<std::vector<size_t>>>::f(mxGetField(in, i, "transfer"))};
    }
    return out;
  }
};

inline mxArray* toMxArray(const fec::Turbo::Scheduling& scheduling)
{
  const char* fieldnames[] = {"activation", "transfer"};
  mxArray* out = mxCreateStructMatrix(1,scheduling.size(), 2, fieldnames);
  for (size_t i = 0; i < scheduling.stages.size(); ++i) {
    mxSetField(out, i, fieldnames[0], toMxArray(scheduling[i].activation));
    mxSetField(out, i, fieldnames[1], toMxArray(scheduling[i].transfer));
  }
  return out;
}


#endif