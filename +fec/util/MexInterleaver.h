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

#ifndef TO_INTERLEAVER_H
#define TO_INTERLEAVER_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>

#include "MexConversion.h"
#include "Structure/Interleaver.h"

template <>
class mxArrayTo<fec::Interleaver> {
public:
  static fec::Interleaver f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    std::vector<size_t> perm = mxArrayTo<std::vector<size_t>>::f(in);
    for (auto & i : perm) {
      i--;
    }
    return fec::Interleaver(perm);
  }
};

inline mxArray* toMxArray(const fec::Interleaver& in) {
  std::vector<size_t> indices(in.outputSize());
  for (size_t i = 0; i < indices.size(); ++i) {
    indices[i] = in[i]+1;
  }
  return toMxArray(indices);
}

#endif
