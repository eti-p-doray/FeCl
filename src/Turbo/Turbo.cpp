/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
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

void Turbo::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
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
    throw std::invalid_argument("Trellis and Permutation count don't match");
  }
  if (encoder.termination_.size() != 1 && encoder.termination_.size() != interleaver_.size()) {
    throw std::invalid_argument("Termination and Permutation count don't match");
  }
  
  msgSize_ = 0;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    if (interleaver_[i].inputSize() > msgSize()) {
      msgSize_ = interleaver_[i].inputSize();
    }
  }
  
  constituents_.clear();
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
  
  systSize_ = 0;
  paritySize_ = 0;
  tailSize_ = 0;
  stateSize_ = 0;
  for (auto & i : constituents()) {
    tailSize_ += i.systTailSize();
    paritySize_ += i.paritySize();
    stateSize_ += i.systSize();
  }
  systSize_ = msgSize_ + systTailSize();
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

void Turbo::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> messageInterl;
  std::vector<BitField<size_t>> parityOut;
  std::vector<BitField<size_t>>::iterator parityOutIt;
  parityOutIt = parity;
  std::copy(msg, msg + msgSize(), parityOutIt);
  auto systTail = parityOutIt + msgSize();
  parityOutIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    interleaver(i).permuteBlock<BitField<size_t>>(msg, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityOutIt, systTail);
    systTail += constituent(i).systTailSize();
    parityOutIt += constituent(i).paritySize();
  }
}

bool Turbo::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> messageInterl;
  std::vector<BitField<size_t>> parityTest;
  std::vector<BitField<size_t>> tailTest;
  std::vector<BitField<size_t>> parityIn;
  std::vector<BitField<size_t>>::const_iterator parityInIt;
  parityInIt = parity;
  auto tailIt = parityInIt + msgSize();
  auto systIt = parityInIt;
  parityInIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    parityTest.resize(constituent(i).paritySize());
    tailTest.resize(constituent(i).systTailSize());
    interleaver(i).permuteBlock<BitField<size_t>,BitField<size_t>>(systIt, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityTest.begin(), tailTest.begin());
    if (!std::equal(parityTest.begin(), parityTest.end(), parityInIt)) {
      return false;
    }
    if (!std::equal(tailTest.begin(), tailTest.end(), tailIt)) {
      return false;
    }
    parityInIt += constituent(i).paritySize();
    tailIt += constituent(i).systTailSize();
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
    auto parityInIt = parityIn + systSize();
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
  auto parityInIt = parityIn + systSize();
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
void fec::Turbo::Structure::group(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut) const
{
  auto msgOutIt = parityOut;
  for (size_t i = 0; i < msgSize(); ++i) {
    *msgOutIt = *parityIn;
    ++msgOutIt;
    ++parityIn;
    auto parityOutIt = parityOut + msgSize() + systTailSize();
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
  auto parityOutIt = parityOut + msgSize() + systTailSize();
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

Permutation Turbo::Structure::createPermutation(const PermuteOptions& options) const
{
  //permutations_ = Permutation
  std::vector<size_t> perms;
  /*for (size_t i = 0; i < length()*trellis().outputSize(); ) {
   for (size_t j = 0; j < options.parityPattern_.size(); ++j) {
   if (options.parityPattern_[j]) {
   perms.push_back(i);
   ++i;
   }
   }
   }*/
  for (size_t i = 0; i < paritySize(); ++i) {
    perms.push_back(i);
  }
  std::vector<size_t> temp(perms.size());
  if (options.bitOrdering_ == Alternate) {
    alternate<size_t>(perms.begin(), temp.begin());
  }
  
  return Permutation(temp, paritySize());
}

template void fec::Turbo::Structure::alternate<BitField<size_t>>(std::vector<BitField<size_t>>::const_iterator parityIn, std::vector<BitField<size_t>>::iterator parityOut) const;
template void fec::Turbo::Structure::alternate<LlrType>(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parityOut) const;

template void fec::Turbo::Structure::group<BitField<size_t>>(std::vector<BitField<size_t>>::const_iterator parityIn, std::vector<BitField<size_t>>::iterator parityOut) const;
template void fec::Turbo::Structure::group<LlrType>(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parityOut) const;
