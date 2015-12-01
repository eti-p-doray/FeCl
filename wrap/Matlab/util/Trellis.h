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

#ifndef WRAP_TRELLIS_H
#define WRAP_TRELLIS_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>

#include "Trellis.h"
#include "Conversion.h"
#include "BitField.h"

template<>
class mxArrayTo<fec::Trellis> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  fec::Trellis operator() (const mxArray* in, size_t i = 0) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Trellis is null");
    }
    return fec::Trellis(mxArrayTo<std::vector<fec::BitField<size_t>>>{msg()}("nextStates")(mxGetField(in, i, "nextStates")),
                            mxArrayTo<std::vector<fec::BitField<size_t>>>{msg()}("outputs")(mxGetField(in, i, "outputs")),
                            log2(mxArrayTo<size_t>{msg()}("numInputSymbols")(mxGetField(in, i, "numInputSymbols"))),
                            log2(mxArrayTo<size_t>{msg()}("numOutputSymbols")(mxGetField(in, i, "numOutputSymbols"))),
                            log2(mxArrayTo<size_t>{msg()}("numStates")(mxGetField(in, i, "numStates"))));
  }
};

inline mxArray* toMxArray(const fec::Trellis& trellis)
{
  const char* fieldnames[] = {"numInputSymbols", "numOutputSymbols", "numStates", "nextStates", "outputs"};
  mxArray* out = mxCreateStructMatrix(1,1,5, fieldnames);
  
  mxSetField(out, 0, fieldnames[0], toMxArray(trellis.inputCount()));
  mxSetField(out, 0, fieldnames[1], toMxArray(trellis.outputCount()));
  mxSetField(out, 0, fieldnames[2], toMxArray(trellis.stateCount()));

  std::vector<fec::BitField<size_t>> nextStates(trellis.inputCount() * trellis.stateCount());
  std::vector<fec::BitField<size_t>> outputs(trellis.inputCount() * trellis.stateCount());
  for (size_t i = 0; i < trellis.stateCount(); ++i) {
    for (fec::BitField<size_t> j = 0; j < trellis.inputCount(); j++) {
      fec::BitField<size_t> input = 0;
      for (int k = 0; k < trellis.inputSize(); ++k) {
        input[k] = j[trellis.inputSize()-k-1];
      }
      nextStates[i+input*trellis.stateCount()] = trellis.getNextState(i, j);
      outputs[i+input*trellis.stateCount()] = 0;
      for (size_t k = 0; k < trellis.outputSize(); k++) {
        outputs[i+input*trellis.stateCount()][trellis.outputSize()-k-1] = trellis.getOutput(i, j).test(k);
      }
    }
  }
  
  mxSetField(out, 0, fieldnames[3], toMxArray(nextStates));
  mxSetField(out, 0, fieldnames[4], toMxArray(outputs));
  return out;
}


#endif