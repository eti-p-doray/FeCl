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
 
 Definition of TurboCode class
 ******************************************************************************/

#include "TurboCode.h"
#include "TurboCodeImpl.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(TurboCode);
BOOST_CLASS_EXPORT_IMPLEMENT(TurboCode::Structure);

const char * TurboCode::get_key() const {
  return boost::serialization::type_info_implementation<TurboCode>::type::get_const_instance().get_key();
}

const char * TurboCode::Structure::get_key() const {
  return boost::serialization::type_info_implementation<TurboCode::Structure>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  TurboCode constructor
 *  \param  codeStructure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
TurboCode::TurboCode(const TurboCode::Structure& structure, int workGroupSize) :
structure_(structure),
Code(&structure_, workGroupSize)
{
}

void TurboCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIn, std::vector<uint8_t>::iterator parityOut) const
{
  structure_.encode(messageIn, parityOut);
}

void TurboCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  TurboCodeImpl worker(structure_);
  worker.appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void TurboCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> extrinsic(n * extrinsicSize(), 0);
  appDecodeNBloc(parityIn, extrinsic.begin(), messageOut, extrinsic.begin(), n);
}

void TurboCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> messageAPosteriori(n * msgSize());
  softOutDecodeNBloc(parityIn, messageAPosteriori.begin(), n);

  for (auto messageIt = messageAPosteriori.begin(); messageIt < messageAPosteriori.end(); ++messageIt, ++messageOut) {
    *messageOut = *messageIt > 0;
  }
}

TurboCode::Structure::Structure(const std::vector<Trellis>& trellis, const std::vector<Interleaver>& interleaver, const std::vector<ConvolutionalCode::TerminationType>& endType, size_t iterationCount, SchedulingType schedulingType, ConvolutionalCode::DecoderType mapType, double gain) :
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
    constituents_.push_back(ConvolutionalCode::Structure(trellis[i], blocSize, endType[i], mapType));
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
  
  extrinsicSize_ = 0;
  switch (this->schedulingType()) {
    default:
    case TurboCode::Serial:
      extrinsicSize_ = msgSize() + msgTailSize();
      break;
      
    case TurboCode::Parallel:
      for (auto & i : constituents()) {
        extrinsicSize_ += i.msgSize() + i.msgTailSize();
      }
      break;
  }
  
  gain_ = gain;
}

void TurboCode::Structure::setDecoderType(ConvolutionalCode::DecoderType type)
{
  for (auto & constituent : constituents_) {
    constituent.setDecoderType(type);
  }
}

void TurboCode::Structure::encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const
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


