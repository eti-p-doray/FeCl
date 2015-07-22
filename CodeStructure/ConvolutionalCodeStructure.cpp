/*******************************************************************************
 *  \file ConvolutionalCodeStructure.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of the ConvolutionalCodeStructure class
 ******************************************************************************/

#include "ConvolutionalCodeStructure.h"

using namespace fec;

ConvolutionalCodeStructure::ConvolutionalCodeStructure(TrellisStructure trellis, size_t blocSize, TrellisEndType endType, DecoderType type) : CodeStructure(blocSize * trellis.inputSize(), blocSize * trellis.outputSize()), trellis_(trellis)
{
  blocSize_ = blocSize;
  trellisEndType_ = endType;
  decoderType_ = type;
  
  switch (trellisEndType_) {
    case ZeroTail:
      paritySize_ += trellis_.stateSize() * trellis.outputSize();
      tailSize_ = trellis_.stateSize();
      break;
      
    default:
    case Truncation:
      tailSize_ = 0;
      break;
  }
}