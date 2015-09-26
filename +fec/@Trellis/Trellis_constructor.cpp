/*******************************************************************************
 This file is part of C3sar.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 C3sar is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 C3sar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
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