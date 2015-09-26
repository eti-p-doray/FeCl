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
 ******************************************************************************/

#include <mex.h>

#include "Structure/Trellis.h"
#include "../util/MexConversion.h"
#include "../util/MexTrellis.h"

using namespace fec;

void Trellis_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  auto constraintLength = mxArrayTo<std::vector<BitField<size_t>>>::f(prhs[0]);
  auto mxGenerator = mxArrayTo<std::vector<BitField<size_t>>>::f(prhs[1]);
  auto feedback = mxArrayTo<std::vector<BitField<size_t>>>::f(prhs[2]);
  
  std::vector<std::vector<BitField<size_t>>> generator(constraintLength.size());
  size_t outputSize = mxGenerator.size() / generator.size();
  if (outputSize * generator.size() != mxGenerator.size()) {
    throw std::invalid_argument("Invalid number of generators");
  }
  for (size_t i = 0; i < generator.size(); ++i) {
    generator[i].resize(outputSize);
    for (size_t j = 0; j < outputSize; ++j) {
      generator[i][j] = mxGenerator[j*outputSize+i];
    }
  }
  
  plhs[0] = toMxArray(Trellis(constraintLength, generator, feedback));
}