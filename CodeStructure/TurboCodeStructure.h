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

#ifndef TURBO_CODE_STRUCTURE_H
#define TURBO_CODE_STRUCTURE_H

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#include "CodeStructure.h"
#include "ConvolutionalCodeStructure.h"
#include "Interleaver.h"

namespace fec {

/**
 *  This class represents a convolutional code structure.
 *  It provides a usefull interface to store and acces the structure information.
 */
class TurboCodeStructure : public CodeStructure {
  friend class ::boost::serialization::access;
public:
  /**
   *  Algorithm used in decoding.
   *  This defines the scheduling of extrinsic communication between code
   *    constituents.
   */
  enum DecoderType {
    Serial,/**< Each constituent tries to decode and gives its extrinsic
              information to the next constituent in a serial behavior. */
    Parallel,/**< Each constituent tries to decode in parallel.
                 The extrinsic information is then combined and shared to every
                 constituents similar to the Belief Propagation algorithm used in ldpc. */
  };
  
  TurboCodeStructure() = default;
  TurboCodeStructure(const std::vector<TrellisStructure>& trellis, const std::vector<Interleaver>& interleaver, size_t iterationCount = 5, DecoderType structureType = Serial, ConvolutionalCodeStructure::DecoderType mapType = ConvolutionalCodeStructure::MaxLogMap);
  virtual ~TurboCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Turbo;}
  
  inline size_t structureCount() const {return structure_.size();}
  inline const std::vector<ConvolutionalCodeStructure>& structures() const {return structure_;}
  inline const std::vector<Interleaver>& interleavers() const {return interleaver_;}
  inline const ConvolutionalCodeStructure& structure(size_t i) const {return structure_[i];}
  inline const Interleaver& interleaver(size_t i) const {return interleaver_[i];}
  inline size_t iterationCount() const {return iterationCount_;}
  inline DecoderType structureType() const {return structureType_;}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & ::BOOST_SERIALIZATION_NVP(structure_);
    ar & ::BOOST_SERIALIZATION_NVP(interleaver_);
    ar & ::BOOST_SERIALIZATION_NVP(structureType_);
    ar & ::BOOST_SERIALIZATION_NVP(iterationCount_);
  }
  
  std::vector<ConvolutionalCodeStructure> structure_;
  std::vector<Interleaver> interleaver_;
  DecoderType structureType_;
  size_t iterationCount_;
};
  
}

#endif
