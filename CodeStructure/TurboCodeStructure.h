/*******************************************************************************
 *  \file TurboCodeStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-17
 *  \version Last update : 2015-06-17
 *
 *  Declaration of the ConvolutionalCodeStructure class
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

/*******************************************************************************
 *  This class represents a convolutional code structure.
 *  It provides a usefull interface to store and acces the structure information.
 ******************************************************************************/
class TurboCodeStructure : public CodeStructure {
  friend class ::boost::serialization::access;
public:
  enum DecoderType {
    Serial,
    Parallel,
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
