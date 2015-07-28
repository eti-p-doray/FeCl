/*******************************************************************************
 *  \file mxArrayToBitMatrix.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of SparseBitMatrix conversion function.
 ******************************************************************************/

#ifndef TO_BITMATRIX_H
#define TO_BITMATRIX_H

#include <memory>
#include <math.h>
#include <boost/container/vector.hpp>
#include <mex.h>
#include <matrix.h>

#include "MexConversion.h"
#include "CodeStructure/BitMatrix.h"

using namespace fec;

template <>
class mxArrayTo<SparseBitMatrix> {
public:
  static SparseBitMatrix f(const mxArray* in) {
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
      
      boost::container::vector<size_t> rowSizes(mxGetM(in), 0);
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t i = jc[j]; i < jc[j+1]; ++i) {
          rowSizes[ir[i]]++;
        }
      }
      
      SparseBitMatrix mat(rowSizes, mxGetN(in));
      
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
      boost::container::vector<size_t> rowSizes(mxGetM(in), 0);
      for (size_t j = 0; j < mxGetN(in); ++j) {
        for (size_t i = 0; i < mxGetM(in); ++i, ++pr) {
          if (*pr != 0) {
            rowSizes[i]++;
          }
        }
      }
      
      SparseBitMatrix mat(rowSizes, mxGetN(in));
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
  
#endif
