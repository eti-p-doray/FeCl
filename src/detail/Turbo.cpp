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

using namespace fec;
using namespace fec::detail;

BOOST_CLASS_EXPORT_IMPLEMENT(Turbo::Structure);

const char * Turbo::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Turbo::Structure>::type::get_const_instance().get_key();
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

void Turbo::Structure::setEncoderOptions(const Turbo::EncoderOptions &encoder)
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
    size_t length = interleaver_[i].outputSize() / encoder.trellis_[j].inputWidth();
    if (length * encoder.trellis_[j].inputWidth() != interleaver_[i].outputSize()) {
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
    tailSize_ += i.tailSize();
    paritySize_ += i.paritySize();
    stateSize_ += i.systSize();
  }
  systSize_ = msgSize_ + tailSize();
  paritySize_ += systSize();
}

void Turbo::Structure::setDecoderOptions(const Turbo::DecoderOptions &decoder)
{
  iterations_ = decoder.iterations_;
  schedulingType_ = decoder.schedulingType_;
  if (schedulingType() == Custom) {
    scheduling_ = decoder.scheduling_;
    for (size_t i = 0; i < scheduling().size(); ++i) {
      if (scheduling()[i].activation.size() != scheduling()[i].transfer.size()) {
        throw std::invalid_argument("Invalid scheduling : activation and transfer not the same size");
      }
      std::sort(scheduling_[i].activation.begin(), scheduling_[i].activation.end());
      for (size_t j = 0; j < scheduling()[i].activation.size(); ++j) {
        if (scheduling()[i].activation[j] > constituentCount()) {
          throw std::invalid_argument("Invalid scheduling : activation of an invalid constituent");
        }
        for (size_t k = 0; k < scheduling()[i].transfer[j].size(); ++k) {
          if (scheduling()[i].transfer[j][k] > constituentCount()) {
            throw std::invalid_argument("Invalid scheduling : transfer from an invalid constituent");
          }
        }
        std::sort(scheduling_[i].transfer[j].begin(), scheduling_[i].transfer[j].end());
      }
    }
  }
  decoderAlgorithm_ = decoder.algorithm_;
  scalingFactor_ = decoder.scalingFactor_;
  if (scalingFactor_.size() == constituentCount()) {
    for (size_t i = 0; i < scalingFactor_.size(); ++i) {
      if (scalingFactor_[i].size() != iterations() && scalingFactor_[i].size() != 1) {
        throw std::invalid_argument("Wrong size for scaling factor");
      }
    }
  } else if (scalingFactor_.size() != 1) {
    throw std::invalid_argument("Wrong size for scaling factor");
  }
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    auto constituentOptions = Convolutional::DecoderOptions().algorithm(decoder.algorithm_).scalingFactor(1.0);
    constituents_[i].setDecoderOptions(constituentOptions);
  }
}

Turbo::DecoderOptions Turbo::Structure::getDecoderOptions() const
{
  return DecoderOptions().iterations(iterations()).scheduling(scheduling()).scheduling(schedulingType()).algorithm(decoderAlgorithm()).scalingFactor(scalingFactor_);
}

double Turbo::Structure::scalingFactor(size_t i, size_t j) const
{
  j %= scalingFactor_.size();
  i %= scalingFactor_[j].size();
  return scalingFactor_[j][i];
}

fec::Permutation Turbo::Structure::puncturing(const PunctureOptions& options) const
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
            for (size_t k = 0; k < constituent(j).trellis().outputWidth(); ++k) {
              size_t parityIdx = i*constituent(j).trellis().outputWidth()+k;
              if (mask_[j+1][parityIdx % mask_[j+1].size()]) {
                perms.push_back(parityBaseIdx+parityIdx);
              }
            }
          }
          parityBaseIdx += constituent(j).paritySize();
        }
      }
      size_t parityIdx = systSize();
      for (size_t i = 0; i < constituentCount(); ++i) {
        parityIdx += constituent(i).paritySize() - constituent(i).tailSize() * constituent(i).trellis().outputWidth();
        size_t tailIdx = 0;
        size_t systTailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize(); ++j) {
          for (size_t k = 0; k < constituent(i).trellis().inputWidth(); ++k) {
            if ((tailMask_[i*2].size() == 0 && (mask_[0][systIdx % mask_[0].size()])) ||
                (tailMask_[i*2].size() != 0 && (tailMask_[i*2][systTailIdx % tailMask_[i*2].size()]))) {
              perms.push_back(systIdx);
            }
            ++systIdx;
            ++systTailIdx;
          }
          for (size_t k = 0; k < constituent(i).trellis().outputWidth(); ++k) {
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
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().inputWidth(); ++j) {
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
        for (size_t j = 0; j < constituent(i).length() * constituent(i).trellis().outputWidth(); ++j) {
          if (mask_[i+1][parityIdx % mask_[i+1].size()]) {
            perms.push_back(idx);
          }
          ++parityIdx;
          ++idx;
        }
        size_t tailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().outputWidth(); ++j) {
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
  
  return Permutation(perms, paritySize());
}
