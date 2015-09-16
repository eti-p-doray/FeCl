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
 
 Definition of Turbo class
 ******************************************************************************/

#include "Turbo.h"
#include "TurboDecoder/TurboDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Turbo);
BOOST_CLASS_EXPORT_IMPLEMENT(Turbo::Structure);

const char * Turbo::get_key() const {
  return boost::serialization::type_info_implementation<Turbo>::type::get_const_instance().get_key();
}

const char * Turbo::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Turbo::Structure>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  Turbo constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
Turbo::Turbo(const Structure& structure,  int workGroupSize) :
structure_(structure),
Codec(&structure_, workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
structure_(encoder, decoder),
Codec(&structure_, workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, int workGroupSize) :
structure_(encoder),
Codec(&structure_, workGroupSize)
{
}

void Turbo::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n) const
{
  auto worker = TurboDecoder::create(structure_);
  worker->decodeBlocks(parity, msg, n);
}

void Turbo::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  auto worker = TurboDecoder::create(structure_);
  worker->soDecodeBlocks(input, output, n);
}


Turbo::Structure::Structure(const EncoderOptions& encoder, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(decoder);
}
Turbo::Structure::Structure(const EncoderOptions& encoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(DecoderOptions());
}

void Turbo::Structure::setEncoderOptions(const fec::Turbo::EncoderOptions &encoder)
{
  interleaver_ = encoder.interleaver_;
  if (encoder.trellis_.size() != 1 && encoder.trellis_.size() != interleaver_.size()) {
    throw std::invalid_argument("Trellis and Interleaver count don't match");
  }
  if (encoder.termination_.size() != 1 && encoder.termination_.size() != interleaver_.size()) {
    throw std::invalid_argument("Termination and Interleaver count don't match");
  }
  
  msgSize_ = 0;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    if (interleaver_[i].inputSize() > msgSize()) {
      msgSize_ = interleaver_[i].inputSize();
    }
  }
  
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    if (interleaver_[i].outputSize() == 0) {
      std::vector<size_t> tmp(msgSize());
      for (size_t j = 0; j < tmp.size(); ++j) {
        tmp[j] = j;
      }
      interleaver_[i] = tmp;
    }
    size_t j = i;
    if (encoder.trellis_.size() == 1) {
      j = 0;
    }
    size_t length = interleaver_[i].outputSize() / encoder.trellis_[j].inputSize();
    if (length * encoder.trellis_[j].inputSize() != interleaver_[i].outputSize()) {
      throw std::invalid_argument("Invalid size for interleaver");
    }
    auto encoderConstituentOptions = Convolutional::EncoderOptions(encoder.trellis_[j], length);
    if (encoder.termination_.size() == 1) {
      encoderConstituentOptions.termination(encoder.termination_[0]);
    }
    else {
      encoderConstituentOptions.termination(encoder.termination_[i]);
    }
    auto decoderConstituentOptions = Convolutional::DecoderOptions().algorithm(decoderAlgorithm_);
    constituents_.push_back(Convolutional::Structure(encoderConstituentOptions, decoderConstituentOptions));
  }
  bitOrdering_ = encoder.bitOrdering_;
  
  systSize_ = 0;
  paritySize_ = 0;
  tailSize_ = 0;
  stateSize_ = 0;
  for (auto & i : constituents()) {
    tailSize_ += i.msgTailSize();
    paritySize_ += i.paritySize();
    stateSize_ += i.systSize();
  }
  systSize_ = msgSize_ + msgTailSize();
  paritySize_ += systSize();
}

void Turbo::Structure::setDecoderOptions(const fec::Turbo::DecoderOptions &decoder)
{
  iterations_ = decoder.iterations_;
  scheduling_ = decoder.scheduling_;
  decoderAlgorithm_ = decoder.algorithm_;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    auto constituentOptions = Convolutional::DecoderOptions().algorithm(decoder.algorithm_);
    constituents_[i].setDecoderOptions(constituentOptions);
  }
}

Turbo::DecoderOptions Turbo::Structure::getDecoderOptions() const
{
  return DecoderOptions().iterations(iterations()).scheduling(scheduling()).algorithm(decoderAlgorithm());
}

void Turbo::Structure::encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity) const
{
  std::vector<BitField<bool>> messageInterl;
  std::vector<BitField<uint8_t>> parityOut;
  std::vector<BitField<uint8_t>>::iterator parityOutIt;
  switch (bitOrdering()) {
    case Alternate:
      parityOut.resize(paritySize());
      parityOutIt = parityOut.begin();
      break;
      
    default:
    case Pack:
      parityOutIt = parity;
      break;
  }
  std::copy(msg, msg + msgSize(), parityOutIt);
  auto systTail = parityOutIt + msgSize();
  parityOutIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    interleaver(i).interleaveBlock<BitField<bool>>(msg, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityOutIt, systTail);
    systTail += constituent(i).msgTailSize();
    parityOutIt += constituent(i).paritySize();
  }
  if (bitOrdering() == Alternate) {
    alternate<BitField<uint8_t>>(parityOut.begin(), parity);
  }
}

bool Turbo::Structure::check(std::vector<BitField<uint8_t>>::const_iterator parity) const
{
  std::vector<BitField<bool>> messageInterl;
  std::vector<BitField<uint8_t>> parityTest;
  std::vector<BitField<uint8_t>> tailTest;
  std::vector<BitField<uint8_t>> parityIn;
  std::vector<BitField<uint8_t>>::const_iterator parityInIt;
  switch (bitOrdering()) {
    case Alternate:
      parityIn.resize(paritySize());
      pack<BitField<uint8_t>>(parity, parityIn.begin());
      parityInIt = parityIn.begin();
      break;
      
    default:
    case Pack:
      parityInIt = parity;
      break;
  }
  auto tailIt = parityInIt + msgSize();
  auto systIt = parityInIt;
  parityInIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    parityTest.resize(constituent(i).paritySize());
    tailTest.resize(constituent(i).msgTailSize());
    interleaver(i).interleaveBlock<BitField<uint8_t>,BitField<bool>>(systIt, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityTest.begin(), tailTest.begin());
    if (!std::equal(parityTest.begin(), parityTest.end(), parityInIt)) {
      return false;
    }
    if (!std::equal(tailTest.begin(), tailTest.end(), tailIt)) {
      return false;
    }
    parityInIt += constituent(i).paritySize();
    tailIt += constituent(i).msgTailSize();
  }
  return true;
}

template <typename T>
void fec::Turbo::Structure::alternate(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut) const
{
  auto msgInIt = parityIn;
  for (size_t i = 0; i < msgSize(); ++i) {
    *parityOut = *msgInIt;
    ++msgInIt;
    ++parityOut;
    auto parityInIt = parityIn + msgSize() + msgTailSize();
    for (size_t j = 0; j < constituentCount(); ++j) {
      if (i < constituent(j).length()) {
        for (size_t k = 0; k < constituent(j).trellis().outputSize(); ++k) {
          *parityOut = parityInIt[i*constituent(j).trellis().outputSize()+k];
          ++parityOut;
        }
      }
      parityInIt += constituent(j).paritySize();
    }
  }
  auto parityInIt = parityIn + msgSize() + msgTailSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    parityInIt += constituent(i).paritySize() - constituent(i).tailSize() * constituent(i).trellis().outputSize();
    for (size_t j = 0; j < constituent(i).tailSize(); ++j) {
      for (size_t k = 0; k < constituent(i).trellis().inputSize(); ++k) {
        *parityOut = *msgInIt;
        ++parityOut;
        ++msgInIt;
      }
      for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
        *parityOut = *parityInIt;
        ++parityOut;
        ++parityInIt;
      }
    }
  }
}

template <typename T>
void fec::Turbo::Structure::pack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut) const
{
  auto msgOutIt = parityOut;
  for (size_t i = 0; i < msgSize(); ++i) {
    *msgOutIt = *parityIn;
    ++msgOutIt;
    ++parityIn;
    auto parityOutIt = parityOut + msgSize() + msgTailSize();
    for (size_t j = 0; j < constituentCount(); ++j) {
      if (i < constituent(j).length()) {
        for (size_t k = 0; k < constituent(j).trellis().outputSize(); ++k) {
          parityOutIt[i*constituent(j).trellis().outputSize()+k] = *parityIn;
          ++parityIn;
        }
      }
      parityOutIt += constituent(j).paritySize();
    }
  }
  auto parityOutIt = parityOut + msgSize() + msgTailSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    parityOutIt += constituent(i).paritySize() - constituent(i).tailSize() * constituent(i).trellis().outputSize();
    for (size_t j = 0; j < constituent(i).tailSize(); ++j) {
      for (size_t k = 0; k < constituent(i).trellis().inputSize(); ++k) {
        *msgOutIt = *parityIn;
        ++parityIn;
        ++msgOutIt;
      }
      for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
        *parityOutIt = *parityIn;
        ++parityOutIt;
        ++parityIn;
      }
    }
  }
}

template void fec::Turbo::Structure::alternate<BitField<uint8_t>>(typename std::vector<BitField<uint8_t>>::const_iterator parityIn, typename std::vector<BitField<uint8_t>>::iterator parityOut) const;
template void fec::Turbo::Structure::alternate<LlrType>(typename std::vector<LlrType>::const_iterator parityIn, typename std::vector<LlrType>::iterator parityOut) const;

template void fec::Turbo::Structure::pack<BitField<uint8_t>>(typename std::vector<BitField<uint8_t>>::const_iterator parityIn, typename std::vector<BitField<uint8_t>>::iterator parityOut) const;
template void fec::Turbo::Structure::pack<LlrType>(typename std::vector<LlrType>::const_iterator parityIn, typename std::vector<LlrType>::iterator parityOut) const;
