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

#ifndef CONVOLUTIONAL_CODE_STRUCTURE_H
#define CONVOLUTIONAL_CODE_STRUCTURE_H

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#include "CodeStructure.h"
#include "TrellisStructure.h"

namespace fec {

/**
 *  This class represents a convolutional code structure.
 *  It provides a usefull interface to store and acces the structure information.
 */
class ConvolutionalCodeStructure : public CodeStructure {
  friend class ::boost::serialization::access;
public:
  /**
   *  Trellis termination types.
   *  This specifies the type of termination at the end of each bloc.
   */
  enum TrellisEndType {
    PaddingTail, /**< The state is brought to zero by implicitly adding new msg bit */
    Truncation /**< The state is forced to zero by truncating the trellis */
  };
  /**
   *  Decoder type
   *  This specifies the decode algorithm used in app decoding.
   */
  enum DecoderType {
    LogMap, /**< No approximation is used and the L-values are computed in logarithmic domain. */
    MaxLogMap,  /**< The maxlog approximation is used */
  };
  
  ConvolutionalCodeStructure() = default;
  ConvolutionalCodeStructure(TrellisStructure trellis, size_t blocSize, TrellisEndType trellisEndType = Truncation, DecoderType type = MaxLogMap);
  virtual ~ConvolutionalCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Convolutional;}
  
  inline size_t blocSize() const {return blocSize_;}
  inline size_t tailSize() const {return tailSize_;}
  inline size_t msgTailSize() const {return tailSize_ * trellis().inputSize();}
  inline TrellisEndType endType() const {return trellisEndType_;}
  inline DecoderType decoderType() const {return decoderType_;}
  inline const TrellisStructure& trellis() const {return trellis_;}
  
  inline void setDecoderType(DecoderType type) {decoderType_ = type;}
  
  BitField<uint64_t> encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & ::BOOST_SERIALIZATION_NVP(trellis_);
    ar & ::BOOST_SERIALIZATION_NVP(trellisEndType_);
    ar & ::BOOST_SERIALIZATION_NVP(decoderType_);
    ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
    ar & ::BOOST_SERIALIZATION_NVP(blocSize_);
  }
  
  TrellisStructure trellis_;
  TrellisEndType trellisEndType_;
  DecoderType decoderType_;
  size_t tailSize_;
  size_t blocSize_;
};
  
}

#endif
