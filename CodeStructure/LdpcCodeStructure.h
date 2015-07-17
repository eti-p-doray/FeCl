/*******************************************************************************
 *  \file LdpcCodeStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Declaration of the LdpcCodeStructure class
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

/*******************************************************************************
 *  This class represents a ldpc code structure.
 *  It provides a usefull interface to store and acces the structure information.
 ******************************************************************************/
class LdpcCodeStructure : public CodeStructure {
  friend class boost::serialization::access;
public:
  enum DecoderType {
    TrueBp,
    MinSumBp,
    ScoreBp,
  };
  
  static SparseBitMatrix gallagerConstruction(size_t n, size_t wc, size_t wr);
  
  LdpcCodeStructure() = default;
  LdpcCodeStructure(const SparseBitMatrix& H, size_t iterationCount = 50, DecoderType type = MinSumBp);
  LdpcCodeStructure(SparseBitMatrix&& H, size_t iterationCount = 50, DecoderType type = MinSumBp);
  virtual ~LdpcCodeStructure() = default;
  
  virtual CodeStructure::Type type() const {return CodeStructure::Ldpc;}
  
  inline const SparseBitMatrix& parityCheck() const {return H_;}
  
  inline const SparseBitMatrix& DC() const {return DC_;}
  inline const SparseBitMatrix& A() const {return A_;}
  inline const SparseBitMatrix& B() const {return B_;}
  inline const SparseBitMatrix& T() const {return T_;}
  
  inline DecoderType decoderType() const {return decoderType_;}
  inline size_t iterationCount() const {return iterationCount_;}
  
  void syndrome(std::vector<uint8_t>::const_iterator parity, std::vector<uint8_t>::iterator syndrome) const;
  bool syndromeCheck(std::vector<uint8_t>::const_iterator parity) const;
  void encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CodeStructure);
    ar & BOOST_SERIALIZATION_NVP(H_);
    ar & BOOST_SERIALIZATION_NVP(DC_);
    ar & BOOST_SERIALIZATION_NVP(T_);
    ar & BOOST_SERIALIZATION_NVP(A_);
    ar & BOOST_SERIALIZATION_NVP(B_);
    ar & BOOST_SERIALIZATION_NVP(decoderType_);
    ar & BOOST_SERIALIZATION_NVP(iterationCount_);
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

#endif
