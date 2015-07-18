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

#include <mex.h>

#include "ConvolutionalCode/ConvolutionalCode.h"
#include "ErrorCorrectingCode.h"

#include "../ecc_export.h"
#include "../MexConversion.h"
#include "../mxArrayToTrellis.h"

const int inputCount = 9;
const int outputCount = 1;

const int MapTypeCount = 2;
const char* const MapTypeEnumeration[MapTypeCount] = {
  "LogMap",
  "MaxLogMap",
};

const int BlocEndTypeCount = 2;
const char* const BlocEndTypeEnumeration[BlocEndTypeCount] = {
  "ZeroTail",
  "Truncation"
};

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
ECC_EXPORT void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ConvolutionalCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ConvolutionalCode_destructor");
  }
  
  TrellisStructure trellis = toTrellisStructure(prhs[0], prhs[1], prhs[2], prhs[3], prhs[4]);
  ConvolutionalCodeStructure codeStructure(trellis, toScalar<size_t>(prhs[5]), toEnum<ConvolutionalCodeStructure::BlocEndType>(prhs[6], BlocEndTypeEnumeration, BlocEndTypeCount), toEnum<ConvolutionalCodeStructure::DecoderType>(prhs[7], MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<ErrorCorrectingCode> code = ErrorCorrectingCode::create(codeStructure, toScalar<size_t>(prhs[8]));
  
  plhs[0] = toMxArray(std::move(code));
}