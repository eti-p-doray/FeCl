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
  /**
   *  Code stucture constructor.
   *  \param  messageSize Size of the msg in each code bloc
   *  \param  paritySize Size of the parity code in each code bloc
   */
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
