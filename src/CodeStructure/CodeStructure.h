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
 
 Declaration of the CodeStructure class
 ******************************************************************************/

#ifndef CODE_STRUCTURE_H
#define CODE_STRUCTURE_H

#include <iostream>
#include <limits>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include "BitField.h"

namespace fec {
  

typedef double LlrType;
const LlrType MAX_LLR = std::numeric_limits<LlrType>::infinity();
const LlrType THRESHOLD_LLR = 1000.0;
  
  /**
   *  Computes the probability (L-value) of a sequence of input L-values
   *  related to a sequence of bits.
   *  The answer is defined as the correlations between the two inputs.
   *  \param  a Sequence of bits as a BitField
   *  \param  b Random access input iterator associated with the sequence of L-values
   *  \return Correlation between the two inputs
   */
  inline LlrType correlationProbability(const BitField<size_t>& a, std::vector<LlrType>::const_iterator b, size_t size) {
    LlrType x = 0;
    for (size_t i = 0; i < size; ++i) {
      if (a.test(i)) {
        x += b[i];
      }
    }
    return x;
  }

/**
 *  This class represents a general code structure
 *  It provides a usefull interface to store and acces the code information.
 */
class CodeStructure {
  friend class boost::serialization::access;
public:
  /**
   *  This enum lists the implemented code structures.
   */
  enum Type {
    Convolutional, /**< Convolutional code following a trellis structure */
    Turbo,  /**< Parallel concatenated convolutional codes */
    Ldpc  /**< Low-density parity check code */
  };
  
  CodeStructure() = default;
  CodeStructure(size_t messageSize, size_t paritySize, size_t extrinsicSize);
  virtual ~CodeStructure() = default;
  
  /**
   *  Access the code structure type as an enumerated type.
   *  \return Code structure type
   */
  virtual Type type() const = 0;
  
  /**
   *  Access the size of the msg in each code bloc.
   *  \return Message size
   */
  inline size_t msgSize() const {return messageSize_;}
  /**
   *  Access the size of the parity bloc in each code bloc.
   *  \return Parity size
   */
  inline size_t paritySize() const {return paritySize_;}
  inline size_t extrinsicSize() const {return extrinsicSize_;}
  
  //virtual void encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const = 0;
  
protected:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(messageSize_);
    ar & BOOST_SERIALIZATION_NVP(paritySize_);
    ar & BOOST_SERIALIZATION_NVP(extrinsicSize_);
  }
  
  size_t messageSize_;/**< Size of the parity bloc in each code bloc. */
  size_t paritySize_;/**< Size of the parity bloc in each code bloc. */
  size_t extrinsicSize_;
};
  
}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::CodeStructure);
  

#endif
