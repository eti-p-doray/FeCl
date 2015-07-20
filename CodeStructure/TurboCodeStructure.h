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
  TurboCodeStructure() = default;
  TurboCodeStructure(TrellisStructure trellis1, TrellisStructure trellis2, Interleaver interleaver, size_t iterationCount = 5, ConvolutionalCodeStructure::BlocEndType endType = ConvolutionalCodeStructure::Truncation, ConvolutionalCodeStructure::DecoderType type = ConvolutionalCodeStructure::MaxLogMap);
  virtual ~TurboCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Turbo;}
  
  inline const ConvolutionalCodeStructure& structure1() const {return structure1_;}
  inline const ConvolutionalCodeStructure& structure2() const {return structure2_;}
  inline const Interleaver& interleaver() const {return interleaver_;}
  inline size_t iterationCount() const {return iterationCount_;}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & ::BOOST_SERIALIZATION_NVP(structure1_);
    ar & ::BOOST_SERIALIZATION_NVP(structure2_);
    ar & ::BOOST_SERIALIZATION_NVP(interleaver_);
    ar & ::BOOST_SERIALIZATION_NVP(iterationCount_);
  }
  
  ConvolutionalCodeStructure structure1_;
  ConvolutionalCodeStructure structure2_;
  Interleaver interleaver_;
  size_t iterationCount_;
};
  
}

#endif
