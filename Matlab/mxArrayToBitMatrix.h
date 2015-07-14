/*******************************************************************************
 *  \file mxArrayToBitMatrix.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of toTrellis conversion function.
 ******************************************************************************/

#ifndef TO_BITMATRIX_H
#define TO_BITMATRIX_H

#include <vector>
#include <memory>
#include <math.h>

#include <mex.h>
#include <matrix.h>

#include <fstream>

#include "../CodeStructure/BitMatrix.h"

SparseBitMatrix toBitMatrix(const mxArray* in) throw(std::bad_cast)
{
  if (mxIsComplex(in) || mxGetData(in) == nullptr) {
    throw std::bad_cast();
  }
  if (mxIsSparse(in)) {
    //std::ofstream os("bou");
    //os << "bou" << std::endl;
    
    auto jc = mxGetJc(in);
    auto ir = mxGetIr(in);
    if (jc == nullptr || ir == nullptr) {
      throw std::bad_cast();
    }
    
    //mexPrintf("n: %i\n", mxGetM(in));
    //mexPrintf("m: %i\n", mxGetN(in));
    //mexPrintf("nz: %i\n", jc[mxGetN(in)]);
    //os << mxGetM(in) << std::endl;
    //os << mxGetM(in) << std::endl;
    //os << jc[mxGetN(in)] << std::endl;
    
    //mexPrintf("1\n");
    
    std::vector<size_t> rowSizes(mxGetM(in), 0);
    for (size_t j = 0; j < mxGetN(in); ++j) {
      for (size_t i = jc[j]; i < jc[j+1]; ++i) {
        rowSizes[ir[i]]++;
      }
    }
    
    SparseBitMatrix mat(rowSizes, mxGetN(in));
    
    //mexPrintf("2\n");
    
    //os << mat.begin()[0][0] << std::endl;
    
    
    /*for (auto row = mat.begin(); row < mat.end(); ++row) {
      for (auto elem = row[0]; elem < row[1];  ++elem) {
        os << int(*elem) << " ";
      }
      //os << std::endl;
    }*/
    
    //mexPrintf("3\n");
    
    auto row = mat.begin();
    for (size_t j = 0; j < mxGetN(in); ++j) {
      for (size_t k = jc[j]; k < jc[j+1]; ++k) {
        size_t i = ir[k];
        row[i].set(j);
        //rowIdx[i]++;
      }
    }
    
    /*for (auto row = mat.begin(); row < mat.end(); ++row) {
      for (auto elem = row[0]; elem < row[1];  ++elem) {
        os << int(*elem) << " ";
      }
      os << std::endl;
    }*/
    
    //mexPrintf("4\n");
    
    //os << mat << std::endl;
    
    //os << "done" << std::endl;
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

#endif
