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
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
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
 *  Turbo constructor.
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
Turbo::Turbo(const Structure& structure,  int workGroupSize) :
Codec(std::unique_ptr<Structure>(new Structure(structure)), workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
Codec(std::unique_ptr<Structure>(new Structure(encoder, decoder)), workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, int workGroupSize) :
Codec(std::unique_ptr<Structure>(new Structure(encoder)), workGroupSize)
{
}

void Turbo::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  auto worker = TurboDecoder::create(structure());
  worker->decodeBlocks(parity, msg, n);
}

void Turbo::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  auto worker = TurboDecoder::create(structure());
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
    paritySize_ += i.innerParitySize();
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
  algorithmOptions_.gain_ = decoder.gain_;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    auto constituentOptions = Convolutional::DecoderOptions().algorithm(decoder.algorithm_).gain(decoder.gain_);
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
    parityOutIt += constituent(i).innerParitySize();
  }
}

bool Turbo::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> messageInterl;
  std::vector<BitField<size_t>> parityTest;
  std::vector<BitField<size_t>> tailTest;
  std::vector<BitField<size_t>>::const_iterator parityInIt;
  parityInIt = parity;
  auto tailIt = parityInIt + msgSize();
  auto systIt = parityInIt;
  parityInIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    parityTest.resize(constituent(i).innerParitySize());
    tailTest.resize(constituent(i).systTailSize());
    interleaver(i).permuteBlock<BitField<size_t>,BitField<size_t>>(systIt, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityTest.begin(), tailTest.begin());
    if (!std::equal(parityTest.begin(), parityTest.end(), parityInIt)) {
      return false;
    }
    if (!std::equal(tailTest.begin(), tailTest.end(), tailIt)) {
      return false;
    }
    parityInIt += constituent(i).innerParitySize();
    tailIt += constituent(i).systTailSize();
  }
  return true;
}

Permutation Turbo::Structure::puncturing(const PunctureOptions& options) const
{
  std::vector<std::vector<bool>> mask_ = options.mask_;
  if (mask_.size() == 0) {
    mask_.resize(constituentCount()+1, {true});
  } else if (mask_.size() == constituentCount()+1) {
    for (size_t i = 0; i < constituentCount()+1; ++i) {
      if (mask_[i].size() == 0) {
        mask_[i] = {true};
      }
    }
  } else {
    throw std::invalid_argument("Invalid size for parity mask");
  }
  std::vector<std::vector<bool>> tailMask_ = options.tailMask_;
  if (tailMask_.size() == 0) {
    tailMask_.resize(constituentCount()*2, {});
  } else if (tailMask_.size() == constituentCount()*2) {

  } else {
    throw std::invalid_argument("Invalid size for tail mask");
  }
  
  
  std::vector<size_t> perms;
  switch (options.bitOrdering_) {
    case Alternate: {
      size_t systIdx = 0;
      for (size_t i = 0; i < msgSize(); ++i) {
        if (mask_[0][systIdx % mask_[0].size()]) {
          perms.push_back(systIdx);
        }
        ++systIdx;
        size_t parityBaseIdx = systSize();
        for (size_t j = 0; j < constituentCount(); ++j) {
          if (i < constituent(j).length()) {
            for (size_t k = 0; k < constituent(j).trellis().outputSize(); ++k) {
              size_t parityIdx = i*constituent(j).trellis().outputSize()+k;
              if (mask_[j+1][parityIdx % mask_[j+1].size()]) {
                perms.push_back(parityBaseIdx+parityIdx);
              }
            }
          }
          parityBaseIdx += constituent(j).innerParitySize();
        }
      }
      size_t parityIdx = systSize();
      for (size_t i = 0; i < constituentCount(); ++i) {
        parityIdx += constituent(i).innerParitySize() - constituent(i).tailSize() * constituent(i).trellis().outputSize();
        size_t tailIdx = 0;
        size_t systTailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize(); ++j) {
          for (size_t k = 0; k < constituent(i).trellis().inputSize(); ++k) {
            if ((tailMask_[i*2].size() == 0 && (mask_[0][systIdx % mask_[0].size()])) ||
                (tailMask_[i*2].size() != 0 && (tailMask_[i*2][systTailIdx % tailMask_[i*2].size()]))) {
              perms.push_back(systIdx);
            }
            ++systIdx;
            ++systTailIdx;
          }
          for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
            if ((tailMask_[i*2+1].size() == 0 && (mask_[i+1][parityIdx % mask_[i+1].size()])) ||
                (tailMask_[i*2+1].size() != 0 && (tailMask_[i*2+1][tailIdx % tailMask_[i*2+1].size()]))) {
              perms.push_back(parityIdx);
            }
            ++parityIdx;
            ++tailIdx;
          }
        }
      }
      break;
    }
      
    case Group: {
      size_t idx = 0;
      for (size_t i = 0; i < msgSize(); ++i) {
        if (mask_[0][idx % mask_[0].size()]) {
          perms.push_back(idx);
        }
        ++idx;
      }
      for (size_t i = 0; i < constituentCount(); ++i) {
        size_t tailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().inputSize(); ++j) {
          if ((tailMask_[i*2].size() == 0 && (mask_[0][idx % mask_[0].size()])) ||
              (tailMask_[i*2].size() != 0 && (tailMask_[i*2][tailIdx % tailMask_[i*2].size()]))) {
            perms.push_back(idx);
          }
          ++idx;
          ++tailIdx;
        }
      }
      for (size_t i = 0; i < constituentCount(); ++i) {
        size_t parityIdx = 0;
        for (size_t j = 0; j < constituent(i).length() * constituent(i).trellis().outputSize(); ++j) {
          if (mask_[i+1][parityIdx % mask_[i+1].size()]) {
            perms.push_back(idx);
          }
          ++parityIdx;
          ++idx;
        }
        size_t tailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().outputSize(); ++j) {
          if ((tailMask_[i*2+1].size() == 0 && (mask_[i+1][parityIdx % mask_[i+1].size()])) ||
              (tailMask_[i*2+1].size() != 0 && (tailMask_[i*2+1][tailIdx % tailMask_[i*2+1].size()]))) {
            perms.push_back(idx);
          }
          ++idx;
          ++parityIdx;
          ++tailIdx;
        }
      }
      break;
    }
      
    default:
      break;
  }
  
  return Permutation(perms, innerParitySize());
}
