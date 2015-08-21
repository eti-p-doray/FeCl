/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of the ConvolutionalCodeStructure class
 ******************************************************************************/

#include "TurboCodeStructure.h"

using namespace fec;

TurboCodeStructure::TurboCodeStructure(const std::vector<TrellisStructure>& trellis, const std::vector<Interleaver>& interleaver, const std::vector<ConvolutionalCodeStructure::TrellisEndType>& endType, size_t iterationCount, SchedulingType schedulingType, ConvolutionalCodeStructure::DecoderType mapType, double gain) :
  interleaver_(interleaver)
{
  schedulingType_ = schedulingType;
  if (trellis.size() != interleaver.size() || interleaver.size() != endType.size()) {
    throw std::invalid_argument("Trellis count, Interleaver count and trellis termination count don't match");
  }
  for (size_t i = 0; i < trellis.size(); ++i) {
    size_t blocSize = interleaver[i].dstSize() / trellis[i].inputSize();
    if (blocSize * trellis[i].inputSize() != interleaver[i].dstSize()) {
      throw std::invalid_argument("Invalid size for interleaver");
    }
    constituents_.push_back(ConvolutionalCodeStructure(trellis[i], blocSize, endType[i], mapType));
  }
  
  iterationCount_ = iterationCount;
  
  messageSize_ = 0;
  paritySize_ = 0;
  tailSize_ = 0;
  for (size_t i = 0; i < constituents_.size(); ++i) {
    tailSize_ += constituent(i).msgTailSize();
    paritySize_ += constituents_[i].paritySize();
    if (interleaver_[i].srcSize() > msgSize()) {
      messageSize_ = interleaver_[i].srcSize();
    }
  }
  paritySize_ += msgSize() + msgTailSize();
  
  gain_ = gain;
}

void TurboCodeStructure::setDecoderType(ConvolutionalCodeStructure::DecoderType type)
{
  for (auto & constituent : constituents_) {
    constituent.setDecoderType(type);
  }
}

void TurboCodeStructure::encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const
{
  std::copy(msg, msg + msgSize(), parity);
  parity += msgSize();
  auto parityIt = parity + msgTailSize();
  std::vector<uint8_t> messageInterl;
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    interleaver(i).interleaveBloc<uint8_t>(msg, messageInterl.begin());
    BitField<uint64_t> tail = constituent(i).encode(messageInterl.begin(), parityIt);
    for (size_t j = 0; j < constituent(i).msgTailSize(); ++j) {
      parity[j] = tail.test(j);
    }
    parity += constituent(i).msgTailSize();
    parityIt += constituent(i).paritySize();
  }
}

