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

#include "Convolutional.h"

using namespace fec;
using namespace fec::detail;

BOOST_CLASS_EXPORT_IMPLEMENT(Convolutional::Structure);

const char * Convolutional::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Convolutional::Structure>::type::get_const_instance().get_key();
}

Convolutional::Structure::Structure(const EncoderOptions& encoder, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(decoder);
}

Convolutional::Structure::Structure(const EncoderOptions& encoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(DecoderOptions());
}

void Convolutional::Structure::setEncoderOptions(const EncoderOptions& encoder)
{
  trellis_ = encoder.trellis_;
  length_ = encoder.length_;
  termination_ = encoder.termination_;
  
  msgSize_ = length_ * trellis_.inputSize();
  systSize_ = length_ * trellis_.inputSize();
  paritySize_ = length_ * trellis_.outputSize();
  stateSize_ = 0;
  switch (termination_) {
    case Trellis::Tail:
      tailSize_ = trellis_.stateSize();
      paritySize_ += trellis_.stateSize() * trellis_.outputSize();
      systSize_ += trellis_.stateSize() * trellis_.inputSize();
      break;
      
    default:
    case Trellis::Truncate:
      tailSize_ = 0;
      break;
  }
}

void Convolutional::Structure::setDecoderOptions(const DecoderOptions& decoder)
{
  decoderAlgorithm_ = decoder.algorithm_;
  scalingFactor_ = decoder.scalingFactor_;
}

Convolutional::DecoderOptions Convolutional::Structure::getDecoderOptions() const
{
  return DecoderOptions().algorithm(decoderAlgorithm_).scalingFactor(scalingFactor_);
}

void Convolutional::Structure::encode(std::vector<fec::BitField<size_t>>::const_iterator msg, std::vector<fec::BitField<size_t>>::iterator parity) const
{
  size_t state = 0;
  
  for (int j = 0; j < length(); ++j) {
    BitField<size_t> input = 0;
    for (int k = 0; k < trellis().inputSize(); k++) {
      input.set(k, msg[k]);
    }
    msg += trellis().inputSize();
    
    BitField<size_t> output = trellis().getOutput(state, input);
    state = trellis().getNextState(state, input);
    
    for (int k = 0; k < trellis().outputSize(); k++) {
      parity[k] = output.test(k);
    }
    parity  += trellis().outputSize();
  }
  
  switch (termination()) {
    case Trellis::Tail:
      for (int j = 0; j < tailSize(); ++j) {
        int maxCount = -1;
        BitField<size_t> bestInput = 0;
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          int count = weigth(BitField<size_t>(state)) - weigth(nextState);
          if (count > maxCount) {
            maxCount = count;
            bestInput = input;
          }
        }
        BitField<size_t> nextState = trellis().getNextState(state, bestInput);
        BitField<size_t> output = trellis().getOutput(state, bestInput);
        for (int k = 0; k < trellis().outputSize(); ++k) {
          parity[k] = output.test(k);
        }
        parity += trellis().outputSize();
        state = nextState;
      }
      break;
      
    default:
    case Trellis::Truncate:
      state = 0;
      break;
  }
  
  assert(state == 0);
}

bool Convolutional::Structure::check(std::vector<fec::BitField<size_t>>::const_iterator parity) const
{
  size_t state = 0;
  for (int j = 0; j < length()+tailSize(); ++j) {
    bool found = false;
    for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
      BitField<size_t> output = trellis().getOutput(state, input);
      bool equal = true;
      for (int k = 0; k < trellis().outputSize(); ++k) {
        if (output[k] != parity[k]) {
          equal = false;
          break;
        }
      }
      if (equal == true) {
        found = true;
        state = trellis().getNextState(state, input);
        break;
      }
    }
    if (found == false) {
      return false;
    }
    parity += trellis().outputSize();
  }
  switch (termination()) {
    case Trellis::Tail:
      return (state == 0);
      
    default:
    case Trellis::Truncate:
      return true;
  }
}

void Convolutional::Structure::encode(std::vector<fec::BitField<size_t>>::const_iterator msg, std::vector<fec::BitField<size_t>>::iterator parity, std::vector<fec::BitField<size_t>>::iterator tail) const
{
  size_t state = 0;
  
  for (int j = 0; j < length(); ++j) {
    BitField<size_t> input = 0;
    for (int k = 0; k < trellis().inputSize(); k++) {
      input.set(k, msg[k]);
    }
    msg += trellis().inputSize();
    
    BitField<size_t> output = trellis().getOutput(state, input);
    state = trellis().getNextState(state, input);
    
    for (int k = 0; k < trellis().outputSize(); k++) {
      parity[k] = output.test(k);
    }
    parity  += trellis().outputSize();
  }
  
  switch (termination()) {
    case Trellis::Tail:
      for (int j = 0; j < tailSize(); ++j) {
        int maxCount = -1;
        BitField<size_t> bestInput = 0;
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          int count = weigth(BitField<size_t>(state)) - weigth(nextState);
          if (count > maxCount) {
            maxCount = count;
            bestInput = input;
          }
        }
        BitField<size_t> nextState = trellis().getNextState(state, bestInput);
        BitField<size_t> output = trellis().getOutput(state, bestInput);
        for (int k = 0; k < trellis().outputSize(); ++k) {
          parity[k] = output.test(k);
        }
        for (int k = 0; k < trellis().inputSize(); ++k) {
          tail[k] = bestInput.test(k);
        }
        parity += trellis().outputSize();
        tail += trellis().inputSize();
        state = nextState;
      }
      break;
      
    default:
    case Trellis::Truncate:
      state = 0;
      break;
  }
  
  assert(state == 0);
}

fec::Permutation Convolutional::Structure::puncturing(const PunctureOptions& options) const
{
  std::vector<size_t> perms;
  size_t systIdx = 0;
  for (size_t i = 0; i < length() * trellis().outputSize(); ++i) {
    if (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()]) {
      perms.push_back(systIdx);
    }
    ++systIdx;
  }
  for (size_t i = 0; i < tailSize()*trellis().outputSize(); ++i) {
    if ((options.tailMask_.size() == 0 && (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()])) ||
        (options.tailMask_.size() != 0 && (options.tailMask_[systIdx % options.tailMask_.size()]))) {
      perms.push_back(systIdx);
    }
    ++systIdx;
  }
  
  return Permutation(perms, paritySize());
}
