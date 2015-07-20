/*******************************************************************************
 *  \file TurboCode_constructor.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-07-15
 *
 *  Definition of TurboCode_constructor mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include "TurboCode/TurboCode.h"
#include "Code.h"

#include "../MexConversion.h"
#include "../mxArrayToTrellis.h"
#include "../mxArrayToInterleaver.h"

const int inputCount = 15;
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
 *  \param  prhs[0] [in]  trellis1 state table
 *  \param  prhs[1] [in]  trellis1 output table
 *  \param  prhs[2] [in]  trellis1 input count
 *  \param  prhs[3] [in]  trellis1 output count
 *  \param  prhs[4] [in]  trellis1 state count
 *  \param  prhs[5] [in]  trellis2 state table
 *  \param  prhs[6] [in]  trellis2 output table
 *  \param  prhs[7] [in]  trellis2 input count
 *  \param  prhs[8] [in]  trellis2 output count
 *  \param  prhs[9] [in]  trellis2 state count
 *  \param  prhs[10] [in]  Interleaver
 *  \param  prhs[11] [in]  Maximum nomber of iterations
 *  \param  prhs[12] [in]  trellis termination type
 *  \param  prhs[13] [in]  decoder algorithm type
 *  \param  prhs[14] [in]  work group size - for parralelisation
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in TurboCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in TurboCode_destructor");
  }
  
  fec::TrellisStructure trellis1 = toTrellisStructure(prhs[0], prhs[1], prhs[2], prhs[3], prhs[4]);
  fec::TrellisStructure trellis2 = toTrellisStructure(prhs[5], prhs[6], prhs[7], prhs[8], prhs[9]);
  fec::TurboCodeStructure codeStructure(trellis1, trellis2, toInterleaver(prhs[10]), toScalar<size_t>(prhs[11]), toEnum<fec::ConvolutionalCodeStructure::BlocEndType>(prhs[12], BlocEndTypeEnumeration, BlocEndTypeCount), toEnum<fec::ConvolutionalCodeStructure::DecoderType>(prhs[13], MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure, toScalar<size_t>(prhs[14]));
  
  plhs[0] = toMxArray(std::move(code));
}