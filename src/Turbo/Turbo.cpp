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
 *  \param  codeStructure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
Turbo::Turbo(const Turbo::Structure& structure, int workGroupSize) :
structure_(structure),
Code(&structure_, workGroupSize)
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


Turbo::Structure::Structure(const EncoderOptions& encode, const DecoderOptions& decode)
{
  interleaver_ = encode.interleaver_;
  if (encode.trellis_.size() != interleaver_.size()) {
    throw std::invalid_argument("Trellis and Interleaver count don't match");
  }
  if (encode.terminationType_.size() != 1 && encode.terminationType_.size() != interleaver_.size()) {
    throw std::invalid_argument("Termination and Interleaver count don't match");
  }
  
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    size_t length = interleaver_[i].outputSize() / encode.trellis_[i].inputSize();
    if (length * encode.trellis_[i].inputSize() != interleaver_[i].outputSize()) {
      throw std::invalid_argument("Invalid size for interleaver");
    }
    auto encoder = Convolutional::EncoderOptions(encode.trellis_[i], length);
    auto decoder = Convolutional::DecoderOptions().decoderType(decode.decoderType_);
    if (encode.terminationType_.size() == 1) {
      encoder.termination(encode.terminationType_[i]);
    }
    else {
      encoder.termination(encode.terminationType_[i]);
    }
    constituents_.push_back(Convolutional::Structure(encoder, decoder));
  }
  
  iterationCount_ = decode.iterationCount_;
  schedulingType_ = decode.schedulingType_;
  decoderType_ = decode.decoderType_;
  
  msgSize_ = 0;
  systSize_ = 0;
  paritySize_ = 0;
  tailSize_ = 0;
  for (size_t i = 0; i < constituents_.size(); ++i) {
    tailSize_ += constituent(i).msgTailSize();
    paritySize_ += constituents_[i].paritySize();
    if (interleaver_[i].inputSize() > msgSize()) {
      msgSize_ = interleaver_[i].inputSize();
    }
  }
  systSize_ = msgSize_ + msgTailSize();
  paritySize_ += systSize();
  
  stateSize_ = 0;
  for (auto & i : constituents()) {
    stateSize_ += i.systSize();
  }
}

void Turbo::Structure::encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity) const
{
  std::copy(msg, msg + msgSize(), parity);
  parity += msgSize();
  auto parityIt = parity + msgTailSize();
  std::vector<BitField<bool>> messageInterl;
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    interleaver(i).interleaveBlock<BitField<bool>>(msg, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityIt, parity);
    parity += constituent(i).msgTailSize();
    parityIt += constituent(i).paritySize();
  }
}

bool Turbo::Structure::check(std::vector<BitField<uint8_t>>::const_iterator parity) const
{
  std::vector<BitField<bool>> messageInterl;
  std::vector<BitField<uint8_t>> parityTest;
  std::vector<BitField<uint8_t>> tailTest;
  auto parityIt = parity + systSize();
  auto tailIt = parity + msgSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    parityTest.resize(constituent(i).paritySize());
    tailTest.resize(constituent(i).msgTailSize());
    interleaver(i).interleaveBlock<BitField<uint8_t>,BitField<bool>>(parity+systSize(), messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityTest.begin(), tailTest.begin());
    if (!std::equal(parityTest.begin(), parityTest.end(), parityIt)) {
      return false;
    }
    if (!std::equal(tailTest.begin(), tailTest.end(), tailIt)) {
      return false;
    }
    parityIt += constituent(i).paritySize();
    tailIt += constituent(i).msgTailSize();
  }
  return true;
}

template <typename T>
void fec::Turbo::Structure::alternate(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut)
{
  auto msgInIt = parityIn;
  auto parityInIt = parityIn + msgSize() + msgTailSize();
  for (size_t i = 0; i < msgSize(); ++i) {
    *parityOut = *msgInIt;
    ++msgInIt;
    ++parityOut;
    for (size_t j = 0; j < constituentCount(); ++j) {
      if (i < constituent(i).length()) {
        for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
          *parityOut = parityInIt[j * constituent(i).paritySize()];
          ++parityOut;
          ++parityInIt;
        }
      }
    }
  }
  parityInIt = parityIn + msgSize() + msgTailSize();
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
void fec::Turbo::Structure::pack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut)
{
  auto msgOutIt = parityOut;
  auto parityOutIt = parityOut + msgSize() + msgTailSize();
  for (size_t i = 0; i < msgSize(); ++i) {
    *msgOutIt = *parityIn;
    ++msgOutIt;
    ++parityIn;
    for (size_t j = 0; j < constituentCount(); ++j) {
      if (i < constituent(i).length()) {
        for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
          parityOutIt[j * constituent(i).paritySize()] = *parityIn;
          ++parityIn;
          ++parityOutIt;
        }
      }
    }
  }
  parityOutIt = parityOut + msgSize() + msgTailSize();
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

template void fec::Turbo::Structure::alternate<BitField<bool>>(typename std::vector<BitField<bool>>::const_iterator parityIn, typename std::vector<BitField<bool>>::iterator parityOut);
template void fec::Turbo::Structure::alternate<LlrType>(typename std::vector<LlrType>::const_iterator parityIn, typename std::vector<LlrType>::iterator parityOut);

template void fec::Turbo::Structure::pack<BitField<bool>>(typename std::vector<BitField<bool>>::const_iterator parityIn, typename std::vector<BitField<bool>>::iterator parityOut);
template void fec::Turbo::Structure::pack<LlrType>(typename std::vector<LlrType>::const_iterator parityIn, typename std::vector<LlrType>::iterator parityOut);
