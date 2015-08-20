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
 
 Declaration of the LdpcCodeStructure class
 ******************************************************************************/

#ifndef LDPC_CODE_STRUCTURE_H
#define LDPC_CODE_STRUCTURE_H

#include <random>
#include <chrono>
#include <algorithm>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "CodeStructure.h"
#include "BitMatrix.h"

namespace fec {

/**
 *  This class represents a ldpc code structure.
 *  It provides a usefull interface to store and acces the structure information.
 */
class LdpcCodeStructure : public CodeStructure {
  friend class ::boost::serialization::access;
public:
  /**
   *  Decoder type
   *  This specifies the decode algorithm used in decoding.
   */
  enum DecoderType {
    TrueBp, /**< No approximation is used and the boxplus formula is computed using standard implementation */
    MinSumBp, /**< The minsum approximation is used instead of the boxplus formula */
  };
  
  static SparseBitMatrix gallagerConstruction(size_t n, size_t wc, size_t wr);
  
  LdpcCodeStructure() = default;
  LdpcCodeStructure(const SparseBitMatrix& H, size_t iterationCount = 50, DecoderType type = MinSumBp);
  LdpcCodeStructure(SparseBitMatrix&& H, size_t iterationCount = 50, DecoderType type = MinSumBp);
  virtual ~LdpcCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Ldpc;}
  
  inline const SparseBitMatrix& parityCheck() const {return H_;}
  
  inline DecoderType decoderType() const {return decoderType_;}
  inline size_t iterationCount() const {return iterationCount_;}
  
  inline void setIterationCount(size_t count) {iterationCount_ = count;}
  
  void syndrome(std::vector<uint8_t>::const_iterator parity, std::vector<uint8_t>::iterator syndrome) const;
  bool check(std::vector<uint8_t>::const_iterator parity) const;
  void encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & ::BOOST_SERIALIZATION_NVP(H_);
    ar & ::BOOST_SERIALIZATION_NVP(DC_);
    ar & ::BOOST_SERIALIZATION_NVP(T_);
    ar & ::BOOST_SERIALIZATION_NVP(A_);
    ar & ::BOOST_SERIALIZATION_NVP(B_);
    ar & ::BOOST_SERIALIZATION_NVP(decoderType_);
    ar & ::BOOST_SERIALIZATION_NVP(iterationCount_);
  }
  
  void computeGeneratorMatrix(SparseBitMatrix&& H);
  
  SparseBitMatrix H_;
  SparseBitMatrix DC_;
  SparseBitMatrix T_;
  SparseBitMatrix A_;
  SparseBitMatrix B_;
  
  DecoderType decoderType_;
  size_t iterationCount_;
};
  
}

#endif
