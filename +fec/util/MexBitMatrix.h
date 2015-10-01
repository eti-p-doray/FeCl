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

#ifndef TO_BITMATRIX_H
#define TO_BITMATRIX_H

#include <memory>
#include <math.h>
#include <vector>
#include <mex.h>
#include <matrix.h>

#include "Structure/BitMatrix.h"
#include "MexConversion.h"
#include "MexBitField.h"

template <>
class mxArrayTo<fec::SparseBitMatrix> {
public:
  static fec::SparseBitMatrix f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    if (mxIsSparse(in)) {
      
      auto jc = mxGetJc(in);
      auto ir = mxGetIr(in);
      if (jc == nullptr || ir == nullptr) {
        throw std::invalid_argument("Invalid data");
      }
      
      std::vector<size_t> rowSizes(mxGetM(in), 0);
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t i = jc[j]; i < jc[j+1]; ++i) {
          rowSizes[ir[i]]++;
        }
      }
      
      fec::SparseBitMatrix mat(rowSizes, mxGetN(in));
      
      auto row = mat.begin();
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t k = jc[j]; k < jc[j+1]; ++k) {
          size_t i = ir[k];
          row[i].set(j);
        }
      }
      return mat;
    }
    else {
      double* pr = mxGetPr(in);
      std::vector<size_t> rowSizes(mxGetM(in), 0);
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t i = 0; i < mxGetM(in); ++i, ++pr) {
          if (*pr != 0) {
            rowSizes[i]++;
          }
        }
      }
      
      fec::SparseBitMatrix mat(rowSizes, mxGetN(in));
      pr = mxGetPr(in);
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t i = 0; i < mxGetM(in); ++i, ++pr) {
          if (*pr != 0) {
            mat[i].set(j);
          }
        }
      }
      return mat;
    }

  }
};

inline mxArray* toMxArray(fec::SparseBitMatrix mat) {
  mat = mat.transpose();
  
  mxArray* out = mxCreateSparseLogicalMatrix(mat.cols(), mat.rows(), mat.size());
  auto jc = mxGetJc(out);
  auto ir = mxGetIr(out);
  auto data = reinterpret_cast<mxLogical*>(mxGetData(out));
  
  jc[0] = 0;
  for (size_t i = 0; i < mat.rows(); ++i) {
    jc[i+1] = jc[i] + mat[i].size();
  }
  
  for (size_t i = 0; i < mat.size(); ++i) {
    ir[i] = mat.at(i);
  }
  
  std::fill(data, data+mat.size(), true);

  return out;
}

#endif
