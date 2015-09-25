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
 
 Definition of SparseBitMatrix conversion function.
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
