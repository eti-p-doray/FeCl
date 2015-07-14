/*******************************************************************************
 *  \file ConvolutionalCode_constructor.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Definition of ConvolutionalCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>
#include <vector>
#include <random>

#include <mex.h>

#include "../../LdpcCode/LdpcCode.h"
#include "../../ErrorCorrectingCode.h"

#include "../MexConversion.h"
#include "../mxArrayToBitMatrix.h"

const int inputCount = 4;
const int outputCount = 1;

const int BpTypeCount = 3;
const char* const BpTypeEnumeration[BpTypeCount] = {
  "TrueBp",
  "MinSumBp",
  "ScoreBp",
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& a)
{
  for (uint64_t i = 0; i < a.size(); i++) {
    os << float(a[i]) << " ";
  }
  return os;
}

/*******************************************************************************
 *  This is the implementation of the constructor in the ConvolutionalCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  trellis state table
 *  \param  prhs[1] [in]  trellis output table
 *  \param  prhs[2] [in]  trellis input count
 *  \param  prhs[3] [in]  trellis output count
 *  \param  prhs[4] [in]  trellis state count
 *  \param  prhs[5] [in]  blocSize
 *  \param  prhs[6] [in]  trellis termination type
 *  \param  prhs[7] [in]  decoder algorithm type
 *  \param  prhs[7] [in]  work group size - for parralelisation
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in LdpcCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in LdpcCode_constructor");
  }
  LdpcCodeStructure codeStructure(toBitMatrix(prhs[0]), toScalar<size_t>(prhs[1]), toEnum<LdpcCodeStructure::DecoderType>(prhs[2], BpTypeEnumeration, BpTypeCount));
  std::unique_ptr<ErrorCorrectingCode> code = ErrorCorrectingCode::create(codeStructure, toScalar<size_t>(prhs[3]));
  
  plhs[0] = toMxArray(std::move(code));
}