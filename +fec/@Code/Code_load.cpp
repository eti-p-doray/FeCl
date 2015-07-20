/*******************************************************************************
 *  \file ErrorCorrectingCode_load.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-07-15
 *
 *  Definition of ErrorCorrectingCode_load mex function
 ******************************************************************************/

#include <memory>
#include <stdexcept>

#include <mex.h>

#include <boost/serialization/export.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "Code.h"

#include "../MexConversion.h"
#include "../MexAllocator.h"

const int inputCount = 1;
const int outputCount = 1;

/*******************************************************************************
 *  This is the implementation of the load method in the ErrorCorrectingCode class.
 *
 *  \param  nlhs    [in]  Number of output
 *  \param  plhs    [out] Array of output mxArray
 *  \param  plhs[0] [out] ErrorCorrectingCode object
 *  \param  nrhs    [in]  Number of input
 *  \param  prhs    [in]  Array of output mxArray
 *  \param  prhs[0] [in]  Serialized object
 ******************************************************************************/
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  if (nrhs != inputCount) {
    throw std::invalid_argument("Wrong argin count in Code_load");
  }
  if (nlhs != outputCount) {
    throw std::invalid_argument("Wrong argout count in Code_load");
  }
  
  std::unique_ptr<fec::Code> code;
  try {
    code = load<fec::Code>(prhs[0]);
    plhs[0] = toMxArray(std::move(code));
  }
  catch (std::exception& e) {
    mexPrintf(e.what());
  }
  
  code.release();
}