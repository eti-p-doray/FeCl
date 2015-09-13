/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
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
 
 Definition of TurboDecoderImpl class
 ******************************************************************************/

#include "TurboDecoderImpl.h"

using namespace fec;

TurboDecoderImpl::TurboDecoderImpl(const Turbo::Structure& structure) : TurboDecoder(structure)
{
}

void TurboDecoderImpl::decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg)
{
  std::copy(parity, parity + structure().systSize(), systIn_.begin());
  std::fill(extrinsic_.begin(), extrinsic_.end(), 0);
  for (size_t i = 0; i < structure().iterationCount(); ++i) {
    if (structure().scheduling() == Turbo::Parallel) {
      parallelSharingUpdate();
    }
    
    auto parityIt = parity + structure().systSize();
    auto extrinsic = extrinsic_.begin();
    for (size_t j = 0; j < structure().constituentCount(); ++j) {
      if (structure().scheduling() == Turbo::Serial) {
        serialSharingUpdate(j);
      }
      
      auto inputInfo = Codec::InputIterator(&structure()).parity(parityIt).syst(extrinsic);
      auto outputInfo = Codec::OutputIterator(&structure()).syst(extrinsic);
      code_[j]->soDecodeBlock(inputInfo, outputInfo);
      
      extrinsic += structure().constituent(j).systSize();
      parityIt += structure().constituent(j).paritySize();
    }
  }
  std::copy(parity, parity+structure().msgSize(), systOut_.begin());
  aPosterioriUpdate();
  
  for (size_t i = 0; i < structure().msgSize(); ++i) {
    msg[i] = systOut_[i] > 0;
  }
}


void TurboDecoderImpl::soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output)
{
  std::copy(input.parity(), input.parity() + structure().systSize(), systIn_.begin());
  if (input.hasSyst()) {
    for (size_t i = 0; i < structure().systSize(); ++i) {
      systIn_[i] += input.syst()[i];
    }
  }
  
  if (input.hasState()) {
    std::copy(input.state(), input.state()+structure().stateSize(), extrinsic_.begin());
  }
  else {
    std::fill(extrinsic_.begin(), extrinsic_.end(), 0);
  }
  
  if (structure().iterationCount() == 0) {
    if (output.hasParity()) {
      std::fill(output.parity()+structure().systSize(), output.parity()+structure().paritySize(), 0);
    }
  }
  
  for (size_t i = 0; i < structure().iterationCount(); ++i) {
    if (structure().scheduling() == Turbo::Parallel) {
      parallelSharingUpdate();
    }
    
    auto parityIn = input.parity() + structure().systSize();
    auto parityOut = output.parity() + structure().systSize();
    auto extrinsic = extrinsic_.begin();
    for (size_t j = 0; j < structure().constituentCount(); ++j) {
      if (structure().scheduling() == Turbo::Serial) {
        serialSharingUpdate(j);
      }
      
      auto inputInfo = Codec::InputIterator(&structure()).parity(parityIn).syst(extrinsic);
      auto outputInfo = Codec::OutputIterator(&structure()).syst(extrinsic);
      if (i == structure().iterationCount()-1 && output.hasParity()) {
        outputInfo.parity(parityOut);
      }
      code_[j]->soDecodeBlock(inputInfo, outputInfo);
      
      extrinsic += structure().constituent(j).systSize();
      parityIn += structure().constituent(j).paritySize();
      parityOut += structure().constituent(j).paritySize();
    }
  }
  std::fill(systOut_.begin(), systOut_.begin() + structure().msgSize(), 0);
  aPosterioriUpdate();

  if (output.hasSyst()) {
    std::copy(systOut_.begin(), systOut_.end(), output.syst());
  }
  if (output.hasParity()) {
    std::copy(systOut_.begin(), systOut_.end(), output.parity());
  }
  if (output.hasState()) {
    std::copy(extrinsic_.begin(), extrinsic_.end(), output.state());
  }
}

void TurboDecoderImpl::aPosterioriUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = systOut_.begin() + structure().msgSize();
  auto syst = systOut_.begin();
  std::fill(systOut_.begin(), systOut_.begin() + structure().msgSize(), 0);
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).msgSize();
    std::copy(extrinsic, extrinsic + structure().constituent(j).msgTailSize(), systTail);
    extrinsic += structure().constituent(j).msgTailSize();
    systTail += structure().constituent(j).msgTailSize();
  }
}

void TurboDecoderImpl::parallelSharingUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto extrinsicTmp = extrinsicBuffer_.begin();
  
  auto systTail = systIn_.begin() + structure().msgSize();
  auto syst = systOut_.begin();
  std::fill(systOut_.begin(), systOut_.begin() + structure().msgSize(), 0);
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      extrinsicTmp[k] = extrinsic[k];
      extrinsic[k] = syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
    extrinsic += structure().constituent(j).msgSize();
    extrinsicTmp += structure().constituent(j).msgSize();
    
    std::copy(systTail, systTail + structure().constituent(j).msgTailSize(), extrinsic);
    
    extrinsic += structure().constituent(j).msgTailSize();
    systTail += structure().constituent(j).msgTailSize();
  }
  std::copy(systIn_.begin(), systIn_.begin() + structure().msgSize(), systOut_.begin());
  
  for (int64_t j = structure().constituentCount()-1; j >= 0; --j) {
    extrinsic -= structure().constituent(j).systSize();
    extrinsicTmp -= structure().constituent(j).msgSize();
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      extrinsic[k] += syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
  }
}

void TurboDecoderImpl::serialSharingUpdate(size_t i)
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = systIn_.begin() + structure().msgSize();
  auto syst = systOut_.begin();
  std::copy(systIn_.begin(), systIn_.begin() + structure().msgSize(), systOut_.begin());
  for (size_t j = 0; j < i; ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).systSize();
    systTail += structure().constituent(j).msgTailSize();
  }
  auto extrinsicTmp = extrinsic;
  std::copy(systTail, systTail + structure().constituent(i).msgTailSize(), extrinsic);
  
  for (size_t j = i+1; j < structure().constituentCount(); ++j) {
    extrinsic += structure().constituent(j).systSize();
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
  }
  
  structure().interleaver(i).template interleaveBlock<LlrType>(syst, extrinsicTmp);
}

//Explicit instantiation
/*template class fec::TurboDecoderImpl<FloatLlrMetrics, LogSum>;
template class fec::TurboDecoderImpl<FloatLlrMetrics, MaxLogSum>;
template class fec::TurboDecoderImpl<FloatLlrMetrics, TableLogSum>;

template class fec::TurboDecoderImpl<FixLlrMetrics, LogSum>;
template class fec::TurboDecoderImpl<FixLlrMetrics, MaxLogSum>;
template class fec::TurboDecoderImpl<FixLlrMetrics, TableLogSum>;*/
