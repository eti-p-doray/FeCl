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

struct Branch {
  BitField<uint16_t> nextState;
  BitField<uint16_t> output;
};

/*******************************************************************************
 *  This class represents a trellis structure.
 *  It provides a usefull interface to access state and outputs of a trellis. 
 ******************************************************************************/
class TrellisStructure {
  friend class boost::serialization::access;
public:
  TrellisStructure() = default;
  TrellisStructure(std::vector<int> nextState, std::vector<int> output, uint8_t inputSize, uint8_t outputSize, uint8_t stateSize);
  TrellisStructure(std::vector<BitField<uint16_t> > constraintLengths, std::vector<std::vector<BitField<uint16_t> > > generator);
  TrellisStructure(std::vector<BitField<uint16_t> > constraintLengths, std::vector<std::vector<BitField<uint16_t> > > generator, std::vector<BitField<uint16_t> > feedback);
  
  TrellisStructure(const TrellisStructure&) = default;
  
  TrellisStructure& operator=(const TrellisStructure& b) = default;
  
  inline int stateSize() const {return stateSize_;}
  inline int stateCount() const {return stateCount_;}
  inline int inputSize() const {return inputSize_;}
  inline int inputCount() const {return inputCount_;}
  inline int outputSize() const {return outputSize_;}
  inline int outputCount() const {return outputCount_;}
  inline int tableSize() const {return tableSize_;}
  
  inline BitField<uint16_t> getNextState(int currentState, int input) const {return nextState_[currentState * inputCount() + input];}
  inline BitField<uint16_t> getOutput(int currentState, int input) const {return output_[currentState * inputCount() + input];}

  inline std::vector<BitField<uint16_t> >::const_iterator beginState() const {return nextState_.begin();}
  inline std::vector<BitField<uint16_t> >::const_iterator endState() const {return nextState_.end();}
  
  inline std::vector<BitField<uint16_t> >::const_iterator beginOutput() const {return output_.begin();}
  inline std::vector<BitField<uint16_t> >::const_iterator endOutput() const {return output_.end();}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(stateSize_);
    ar & BOOST_SERIALIZATION_NVP(inputSize_);
    ar & BOOST_SERIALIZATION_NVP(outputSize_);
    ar & BOOST_SERIALIZATION_NVP(stateCount_);
    ar & BOOST_SERIALIZATION_NVP(inputCount_);
    ar & BOOST_SERIALIZATION_NVP(outputCount_);
    ar & BOOST_SERIALIZATION_NVP(tableSize_);
    ar & BOOST_SERIALIZATION_NVP(nextState_);
    ar & BOOST_SERIALIZATION_NVP(output_);
  }
  
  int stateSize_;
  int inputSize_;
  int outputSize_;
  
  int stateCount_;
  int inputCount_;
  int outputCount_;
  
  int tableSize_;
  
  std::vector<BitField<uint16_t> > nextState_;
  std::vector<BitField<uint16_t> > output_;
};

std::ostream& operator<<(std::ostream& os, const TrellisStructure& trellis);

#endif
