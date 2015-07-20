/*******************************************************************************
 *  \file ConvolutionalCodeStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-28
 *
 *  Declaration of the ConvolutionalCodeStructure class
 ******************************************************************************/

#ifndef CONVOLUTIONAL_CODE_STRUCTURE_H
#define CONVOLUTIONAL_CODE_STRUCTURE_H

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#include "CodeStructure.h"
#include "TrellisStructure.h"

namespace fec {

/*******************************************************************************
 *  This class represents a convolutional code structure.
 *  It provides a usefull interface to store and acces the structure information.
 ******************************************************************************/
class ConvolutionalCodeStructure : public CodeStructure {
  friend class ::boost::serialization::access;
public:
  enum BlocEndType {
    ZeroTail,
    Truncation
  };
  enum DecoderType {
    LogMap,
    MaxLogMap,
  };
  
  ConvolutionalCodeStructure() = default;
  ConvolutionalCodeStructure(TrellisStructure trellis, size_t blocSize, BlocEndType endType = Truncation, DecoderType type = MaxLogMap);
  virtual ~ConvolutionalCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Convolutional;}
  
  inline size_t blocSize() const {return blocSize_;}
  inline size_t tailSize() const {return tailSize_;}
  inline BlocEndType endType() const {return endType_;}
  inline DecoderType decoderType() const {return decoderType_;}
  inline const TrellisStructure& trellis() const {return trellis_;}
  
  static inline LlrType correlationProbability(const BitField<uint16_t>& a, std::vector<LlrType>::const_iterator b, size_t size) {
    LlrType x = 0;
    for (size_t i = 0; i < size; ++i) {
      if (a.test(i)) {
        x += b[i];
      }
    }
    return x;
  }
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & ::BOOST_SERIALIZATION_NVP(trellis_);
    ar & ::BOOST_SERIALIZATION_NVP(endType_);
    ar & ::BOOST_SERIALIZATION_NVP(decoderType_);
    ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
    ar & ::BOOST_SERIALIZATION_NVP(blocSize_);
  }
  
  TrellisStructure trellis_;
  BlocEndType endType_;
  DecoderType decoderType_;
  size_t tailSize_;
  size_t blocSize_;
};
  
}

#endif
