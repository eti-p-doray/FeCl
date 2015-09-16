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

#ifndef TO_TRELLIS_H
#define TO_TRELLIS_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>

#include "Structure/Trellis.h"
#include "MexConversion.h"

template<>
class mxArrayTo<fec::Trellis> {
public:
  static fec::Trellis f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    return fec::Trellis(mxArrayTo<std::vector<size_t>>::f(mxGetField(in, 0, "nextStates")),
                            mxArrayTo<std::vector<size_t>>::f(mxGetField(in, 0, "outputs")),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numInputSymbols"))),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numOutputSymbols"))),
                            log2(mxArrayTo<size_t>::f(mxGetField(in, 0, "numStates"))));
  }
};


#endif