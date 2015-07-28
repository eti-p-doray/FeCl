/*******************************************************************************
 *  \file LdpcCode_constructor.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-07-15
 *
 *  Definition of LdpcCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>
#include <vector>
#include <random>

#include <mex.h>

#include "LdpcCode/LdpcCode.h"
#include "Code.h"

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
std::ostream& operator<<(std::ostream& os, const boost::container::vector<T>& a)
{
  for (uint64_t i = 0; i < a.size(); i++) {
    os << float(a[i]) << " ";
  }
  return os;
}

/*******************************************************************************
 *  This is the implementation of the constructor in the LdpcCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  parityCheck matrix
 *  \param  prhs[1] [in]  Maximum number of iterations
 *  \param  prhs[2] [in]  Decoder algorithm type
 *  \param  prhs[3] [in]  work group size - for parralelisation
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in LdpcCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in LdpcCode_constructor");
  }
  
  fec::LdpcCodeStructure codeStructure(mxArrayTo<SparseBitMatrix>::f(prhs[0]), mxArrayTo<size_t>::f(prhs[1]), mxArrayTo<LdpcCodeStructure::DecoderType>::f(prhs[2],BpTypeEnumeration, BpTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure,mxArrayTo<size_t>::f(prhs[3]));
  
  plhs[0] = toMxArray(std::move(code));
}