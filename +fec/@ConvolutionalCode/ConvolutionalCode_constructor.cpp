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
#include "Code.h"

#include "../MexConversion.h"
#include "../mxArrayToTrellis.h"

const int inputCount = 5;
const int outputCount = 1;

const int MapTypeCount = 2;
const char* const MapTypeEnumeration[MapTypeCount] = {
  "LogMap",
  "MaxLogMap",
};

const int TrellisEndTypeCount = 2;
const char* const TrellisEndTypeEnumeration[TrellisEndTypeCount] = {
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
 *  \param  prhs[1] [in]  blocSize
 *  \param  prhs[2] [in]  trellis termination type
 *  \param  prhs[3] [in]  decoder algorithm type
 *  \param  prhs[4] [in]  work group size - for parralelisation
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in ConvolutionalCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in ConvolutionalCode_destructor");
  }

  fec::TrellisStructure trellis = mxArrayTo<TrellisStructure>::f(prhs[0]);
  fec::ConvolutionalCodeStructure codeStructure(trellis, mxArrayTo<size_t>::f(prhs[1]), mxArrayTo<fec::ConvolutionalCodeStructure::TrellisEndType>::f(prhs[2],TrellisEndTypeEnumeration, TrellisEndTypeCount),
                                                mxArrayTo<fec::ConvolutionalCodeStructure::DecoderType>::f(prhs[3],MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure, mxArrayTo<size_t>::f(prhs[4]));
  
  plhs[0] = toMxArray(std::move(code));
}