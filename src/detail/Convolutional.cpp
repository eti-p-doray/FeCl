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
  inputWidth_ = encoder.msgWidth_;
  if (inputWidth_ == 0) {
    inputWidth_ = trellis().inputWidth();
  }
  if (trellis().inputWidth() % inputWidth_ != 0) {
    throw std::invalid_argument("invalid msg width");
  }
  outputWidth_ = encoder.parityWidth_;
  if (trellis().outputWidth() % outputWidth_ != 0) {
    throw std::invalid_argument("invalid parity width");
  }
  inputLength_ = trellis().inputWidth() / msgWidth();
  outputLength_ = trellis().outputWidth() / parityWidth();
  
  switch (termination_) {
    case Trellis::Tail:
      tailLength_ = trellis_.longestState();
      break;
      
    default:
    case Trellis::Truncate:
      tailLength_ = 0;
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

fec::Permutation Convolutional::Structure::puncturing(const PunctureOptions& options) const
{
  std::vector<size_t> perms;
  size_t systIdx = 0;
  for (size_t i = 0; i < length() * outputLength(); ++i) {
    if (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()]) {
      perms.push_back(systIdx);
    }
    ++systIdx;
  }
  for (size_t i = 0; i < tailLength()*outputLength(); ++i) {
    if ((options.tailMask_.size() == 0 && (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()])) ||
        (options.tailMask_.size() != 0 && (options.tailMask_[systIdx % options.tailMask_.size()]))) {
      perms.push_back(systIdx);
    }
    ++systIdx;
  }
  
  return Permutation(perms, paritySize());
}
