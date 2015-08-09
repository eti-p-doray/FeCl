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

TurboCodeStructure::TurboCodeStructure(const std::vector<TrellisStructure>& trellis, const std::vector<Interleaver>& interleaver, size_t iterationCount, DecoderType structureType, ConvolutionalCodeStructure::DecoderType mapType) :
  interleaver_(interleaver)
{
  structureType_ = structureType;
  if (trellis.size() != interleaver.size()) {
    throw std::invalid_argument("Trellis count and Interleaver count don't match");
  }
  for (size_t i = 0; i < trellis.size(); ++i) {
    size_t blocSize = interleaver[i].dstSize() / trellis[i].inputSize();
    if (blocSize * trellis[i].inputSize() != interleaver[i].dstSize()) {
      throw std::invalid_argument("Invalid size for interleaver");
    }
    structure_.push_back(ConvolutionalCodeStructure(trellis[i], blocSize, ConvolutionalCodeStructure::Truncation, mapType));
  }
  
  iterationCount_ = iterationCount;
  
  messageSize_ = 0;
  paritySize_ = 0;
  for (size_t i = 0; i < structure_.size(); ++i) {
    paritySize_ += structure_[i].paritySize();
    if (interleaver_[i].srcSize() > msgSize()) {
      messageSize_ = interleaver_[i].srcSize();
    }
  }
  paritySize_ += msgSize();
}