/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Definition of toTrellis conversion function.
 ******************************************************************************/

#ifndef MEX_TRELLIS_H
#define MEX_TRELLIS_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>

#include "Structure/Trellis.h"
#include "MexConversion.h"
#include "MexBitField.h"

template<>
class mxArrayTo<fec::Trellis> {
public:
  static fec::Trellis f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    return fec::Trellis(mxArrayTo<std::vector<fec::BitField<size_t>>>::f(mxGetField(in, 0, "nextStates")),
                            mxArrayTo<std::vector<fec::BitField<size_t>>>::f(mxGetField(in, 0, "outputs")),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numInputSymbols"))),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numOutputSymbols"))),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numStates"))));
  }
};

inline mxArray* toMxArray(const fec::Trellis& trellis)
{
  const char* fieldnames[] = {"numInputSymbols", "numOutputSymbols", "numStates", "nextStates", "outputs"};
  mxArray* out = mxCreateStructMatrix(1,1,1, fieldnames);
  
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