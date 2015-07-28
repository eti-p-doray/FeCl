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

const int inputCount = 6;
const int outputCount = 1;

const int StructureTypeCount = 2;
const char* const StructureTypeEnumeration[StructureTypeCount] = {
  "Serial",
  "Parallel",
};

const int MapTypeCount = 2;
const char* const MapTypeEnumeration[MapTypeCount] = {
  "LogMap",
  "MaxLogMap",
};

/*******************************************************************************
 *  This is the implementation of the constructor in the ConvolutionalCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] mexHandle of the created object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  trellis
 *  \param  prhs[1] [in]  Interleaver
 *  \param  prhs[2] [in]  Maximum nomber of iterations
 *  \param  prhs[3] [in]  trellis termination type
 *  \param  prhs[4] [in]  decoder algorithm type
 *  \param  prhs[5] [in]  work group size - for parralelisation
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in TurboCode_constructor");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in TurboCode_destructor");
  }
  
  std::vector<fec::TrellisStructure> trellis = mxCellArrayTo<fec::TrellisStructure>::f(prhs[0]);
  std::vector<fec::Interleaver> interleavers = mxCellArrayTo<fec::Interleaver>::f(prhs[1]);
  
  fec::TurboCodeStructure codeStructure(trellis, interleavers, mxArrayTo<size_t>::f(prhs[2]),
                                        mxArrayTo<fec::TurboCodeStructure::DecoderType>::f(prhs[3], StructureTypeEnumeration, StructureTypeCount),
                                        mxArrayTo<fec::ConvolutionalCodeStructure::DecoderType>::f(prhs[4],MapTypeEnumeration, MapTypeCount));
  std::unique_ptr<fec::Code> code = fec::Code::create(codeStructure, mxArrayTo<size_t>::f(prhs[5]));
  
  plhs[0] = toMxArray(std::move(code));
}