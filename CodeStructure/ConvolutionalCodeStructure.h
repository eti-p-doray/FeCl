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
  /**
   *  Trellis termination types.
   *  This specifies the type of termination at the end of each bloc.
   */
  enum TrellisEndType {
    ZeroTail, /**< The state is brought to zero by implicitly adding new msg bit */
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
  
  /** 
   *  Access the number of branches in one code bloc.
   *  \return Bloc size
   */
  inline size_t blocSize() const {return blocSize_;}
  /** 
   *  Access the size of added msg bit for the trellis termination.
   *  This is zero in the cas of trunction.
   *  \return Tail size
   */
  inline size_t tailSize() const {return tailSize_;}
  /** 
   *  Access the type of the trellis termination.
   *  \return Trellis end type
   */
  inline TrellisEndType endType() const {return trellisEndType_;}
  /** 
   *  Access the algorithm used for app decoding.
   *  \return Decoder type
   */
  inline DecoderType decoderType() const {return decoderType_;}
  /** 
   *  Access the trellis structure object used by the code.
   *  \return Trellis structure
   */
  inline const TrellisStructure& trellis() const {return trellis_;}
  
  /** 
   *  Computes the probability (L-value) of a sequence of input L-values 
   *  related to a sequence of bits.
   *  The answer is defined as the correlations between the two inputs.
   *  \param  a Sequence of bits as a BitField
   *  \param  b Random access input iterator associated with the sequence of L-values
   *  \return Correlation between the two inputs
   */
  static inline LlrType correlationProbability(const BitField<size_t>& a, std::vector<LlrType>::const_iterator b, size_t size) {
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
