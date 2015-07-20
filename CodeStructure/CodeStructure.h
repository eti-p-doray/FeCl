/*******************************************************************************
 *  \file CodeStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-07-08
 *
 *  Declaration of the CodeStructure class
 ******************************************************************************/

#ifndef CODE_STRUCTURE_H
#define CODE_STRUCTURE_H

#include <iostream>
#include <vector>
#include <limits>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

namespace fec {

typedef double LlrType;
const LlrType MAX_LLR = std::numeric_limits<LlrType>::infinity();
const LlrType THRESHOLD_LLR = 10000.0;

struct LlrPdf {
  LlrType mean;
  LlrType var;
};

/*******************************************************************************
 *  This class represents a general code structure
 *  It provides a usefull interface to store and acces the code information.
 ******************************************************************************/
class CodeStructure {
  friend class boost::serialization::access;
public:
  enum Type {Convolutional, Turbo, Ldpc};
  
  CodeStructure() = default;
  CodeStructure(size_t messageSize, size_t paritySize);
  virtual ~CodeStructure() = default;
  
  virtual Type type() const = 0;
  
  inline size_t msgSize() const {return messageSize_;}
  inline size_t paritySize() const {return paritySize_;}
  
protected:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(messageSize_);
    ar & BOOST_SERIALIZATION_NVP(paritySize_);
  }
  
  size_t messageSize_;
  size_t paritySize_;
};
  
}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::CodeStructure);
  

#endif
