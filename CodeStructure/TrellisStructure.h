/*******************************************************************************
 *  \file TrellisStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-12
 *
 *  Declaration of TrellisStructure class
 ******************************************************************************/

#ifndef TRELLIS_STRUCTURE_H
#define TRELLIS_STRUCTURE_H

#include <assert.h>
#include <iostream>

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "BitField.h"

namespace fec {

struct Branch {
  BitField<size_t> nextState;
  BitField<size_t> output;
};

/*******************************************************************************
 *  This class represents a trellis structure.
 *  It provides a usefull size_terface to access state and outputs of a trellis. 
 ******************************************************************************/
class TrellisStructure {
  friend class boost::serialization::access;
public:
  TrellisStructure() = default;
  TrellisStructure(::std::vector<size_t> nextState, ::std::vector<size_t> output, size_t inputSize, size_t outputSize, size_t stateSize);
  TrellisStructure(::std::vector<BitField<size_t> > constraintLengths, ::std::vector<::std::vector<BitField<size_t> > > generator);
  TrellisStructure(::std::vector<BitField<size_t> > constraintLengths, ::std::vector<::std::vector<BitField<size_t> > > generator, ::std::vector<BitField<size_t> > feedback);
  
  TrellisStructure(const TrellisStructure&) = default;
  
  TrellisStructure& operator=(const TrellisStructure& b) = default;
  
  inline size_t stateSize() const {return stateSize_;}
  inline size_t stateCount() const {return stateCount_;}
  inline size_t inputSize() const {return inputSize_;}
  inline size_t inputCount() const {return inputCount_;}
  inline size_t outputSize() const {return outputSize_;}
  inline size_t outputCount() const {return outputCount_;}
  inline size_t tableSize() const {return tableSize_;}
  
  inline BitField<size_t> getNextState(size_t currentState, size_t input) const {return nextState_[currentState * inputCount() + input];}
  inline BitField<size_t> getOutput(size_t currentState, size_t input) const {return output_[currentState * inputCount() + input];}

  inline ::std::vector<BitField<size_t> >::const_iterator beginState() const {return nextState_.begin();}
  inline ::std::vector<BitField<size_t> >::const_iterator endState() const {return nextState_.end();}
  
  inline ::std::vector<BitField<size_t> >::const_iterator beginOutput() const {return output_.begin();}
  inline ::std::vector<BitField<size_t> >::const_iterator endOutput() const {return output_.end();}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & ::BOOST_SERIALIZATION_NVP(stateSize_);
    ar & ::BOOST_SERIALIZATION_NVP(inputSize_);
    ar & ::BOOST_SERIALIZATION_NVP(outputSize_);
    ar & ::BOOST_SERIALIZATION_NVP(stateCount_);
    ar & ::BOOST_SERIALIZATION_NVP(inputCount_);
    ar & ::BOOST_SERIALIZATION_NVP(outputCount_);
    ar & ::BOOST_SERIALIZATION_NVP(tableSize_);
    ar & ::BOOST_SERIALIZATION_NVP(nextState_);
    ar & ::BOOST_SERIALIZATION_NVP(output_);
  }
  
  size_t stateSize_;
  size_t inputSize_;
  size_t outputSize_;
  
  size_t stateCount_;
  size_t inputCount_;
  size_t outputCount_;
  
  size_t tableSize_;
  
  ::std::vector<BitField<size_t> > nextState_;
  ::std::vector<BitField<size_t> > output_;
};

}

std::ostream& operator<<(std::ostream& os, const fec::TrellisStructure& trellis);

#endif
