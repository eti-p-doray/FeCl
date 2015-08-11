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

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

namespace fec {

typedef double LlrType;
const LlrType MAX_LLR = std::numeric_limits<LlrType>::infinity();
const LlrType THRESHOLD_LLR = 10000.0;

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
  CodeStructure(size_t messageSize, size_t paritySize);
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
  
protected:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(messageSize_);
    ar & BOOST_SERIALIZATION_NVP(paritySize_);
  }
  
  size_t messageSize_;/**< Size of the parity bloc in each code bloc. */
  size_t paritySize_;/**< Size of the parity bloc in each code bloc. */
};
  
}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::CodeStructure);
  

#endif
