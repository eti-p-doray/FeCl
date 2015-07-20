/*******************************************************************************
 *  \file TurboCodeStructure.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of the ConvolutionalCodeStructure class
 ******************************************************************************/

#include "TurboCodeStructure.h"

using namespace fec;

TurboCodeStructure::TurboCodeStructure(TrellisStructure trellis1, TrellisStructure trellis2, Interleaver interleaver, size_t iterationCount, ConvolutionalCodeStructure::BlocEndType endType, ConvolutionalCodeStructure::DecoderType type) :
  CodeStructure(interleaver.size(), 0),
  structure1_(trellis1, interleaver.size(), endType, type),
  structure2_(trellis2, interleaver.size(), ConvolutionalCodeStructure::Truncation, type),
  interleaver_(interleaver)
{
  iterationCount_ = iterationCount;
  
  paritySize_ = structure1_.paritySize() + structure2_.paritySize() - msgSize();
}