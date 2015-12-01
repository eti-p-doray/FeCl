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
class mxArrayTo<fec::Turbo::Scheduling> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Turbo::Scheduling operator() (const mxArray* in) const {
    if (in == nullptr) {
      return {};
    }
    
    fec::Turbo::Scheduling out;
    out.resize(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = {mxArrayTo<std::vector<size_t>>{msg()}("activation")(mxGetField(in, i, "activation")), mxArrayTo<std::vector<std::vector<size_t>>>{msg()}("transfer")(mxGetField(in, i, "transfer"))};
      for (size_t j = 0; j < out[i].activation.size(); ++j) {
        out[i].activation[j] --;
        for (size_t k = 0; k < out[i].transfer[j].size(); ++k) {
          out[i].transfer[j][k] --;
        }
      }
    }
    return out;
  }
};

inline mxArray* toMxArray(fec::Turbo::Scheduling scheduling)
{
  const char* fieldnames[] = {"activation", "transfer"};
  mxArray* out = mxCreateStructMatrix(1,scheduling.size(), 2, fieldnames);
  for (size_t i = 0; i < scheduling.size(); ++i) {
    for (size_t j = 0; j < scheduling[i].activation.size(); ++j) {
      scheduling[i].activation[j] ++;
      for (size_t k = 0; k < scheduling[i].transfer[j].size(); ++k) {
        scheduling[i].transfer[j][k] ++;
      }
    }
    mxSetField(out, i, fieldnames[0], toMxArray(scheduling[i].activation));
    mxSetField(out, i, fieldnames[1], toMxArray(scheduling[i].transfer));
  }
  return out;
}


#endif