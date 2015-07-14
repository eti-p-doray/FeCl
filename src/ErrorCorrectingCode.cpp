/*******************************************************************************
 *  \file ErroCorrectingCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Definition of MapCode class
 ******************************************************************************/

#include "ErrorCorrectingCode.h"

#include "ConvolutionalCode/ConvolutionalCode.h"
#include "TurboCode/TurboCode.h"
#include "LdpcCode/LdpcCode.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ErrorCorrectingCode);

std::unique_ptr<ErrorCorrectingCode> ErrorCorrectingCode::create(const CodeStructure& codeStructure, int workGroupdSize)
{
  switch (codeStructure.type()) {
    case CodeStructure::Convolutional:
      return std::unique_ptr<ErrorCorrectingCode>(new ConvolutionalCode(dynamic_cast<const ConvolutionalCodeStructure&>(codeStructure), workGroupdSize));
      break;
      
    case CodeStructure::Turbo:
      return std::unique_ptr<ErrorCorrectingCode>(new TurboCode(dynamic_cast<const TurboCodeStructure&>(codeStructure), workGroupdSize));
      break;
      
    case CodeStructure::Ldpc:
      return std::unique_ptr<ErrorCorrectingCode>(new LdpcCode(dynamic_cast<const LdpcCodeStructure&>(codeStructure), workGroupdSize));
      break;
      
    default:
      break;
  }
  return std::unique_ptr<ErrorCorrectingCode>();
}

ErrorCorrectingCode::ErrorCorrectingCode(int workGroupSize)
{
  workGroupSize_ = workGroupSize;
}

void ErrorCorrectingCode::encodeNBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    encodeBloc(messageIt, parityIt);
    messageIt += messageSize();
    parityIt += paritySize();
  }
}