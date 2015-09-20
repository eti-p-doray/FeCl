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
 
 Declaration of Trellis class
 ******************************************************************************/

#ifndef FEC_TRELLIS_STRUCTURE_H
#define FEC_TRELLIS_STRUCTURE_H

#include <assert.h>
#include <iostream>

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "BitField.h"

namespace fec {

/**
 *  This class represents a trellis structure.
 *  It provides a usefull interface to access state and outputs of a trellis.
 *  The trellis is represented as a lookup table to access 
 *  the next state and the output sequence of a branch given the current state
 *  and the input bit sequence.
 */
class Trellis {
  friend class boost::serialization::access;
public:
  /**
   *  This struct represents branch information.
   */
  struct Branch {
    BitField<size_t> nextState;/**< Final state of the branch */
    BitField<size_t> output;/**< Output symbol sequence associated with the branch */
  };
  
  Trellis() = default;
  Trellis(std::vector<size_t> nextState, std::vector<size_t> output, size_t inputSize, size_t outputSize, size_t stateSize);
  Trellis(std::vector<BitField<size_t> > constraintLengths, std::vector<std::vector<BitField<size_t> > > generator);
  Trellis(std::vector<BitField<size_t> > constraintLengths, std::vector<std::vector<BitField<size_t> > > generator, std::vector<BitField<size_t> > feedback);/**< Not yet implemented */
  
  Trellis(const Trellis&) = default;
  
  Trellis& operator=(const Trellis& b) = default;
  
  /**
   *  Access the state size (register count) of the trellis.
   *  \return State size
   */
  inline size_t stateSize() const {return stateSize_;}
  /**
   *  Access the number of possible states (2^stateSize) of the trellis.
   *  \return State count
   */
  inline size_t stateCount() const {return stateCount_;}
  /**
   *  Access the number of input bits per branch.
   *  \return Input size
   */
  inline size_t inputSize() const {return inputSize_;}
  /**
   *  Access the number of possible configuration of input bits per branch (2^inputSize).
   *  \return Input count
   */
  inline size_t inputCount() const {return inputCount_;}
  /**
   *  Access the number of output symbols per branch.
   *  \return Output size
   */
  inline size_t outputSize() const {return outputSize_;}
  /**
   *  Access the number of possible configuration of output symbols per branch (2^outputSize).
   *  \return Output count
   */
  inline size_t outputCount() const {return outputCount_;}
  /**
   *  Access the size of the state and output table.
   *  This is equal to inputCount * stateCount
   *  \return Table size
   */
  inline size_t tableSize() const {return nextState_.size();}
  
  /**
   *  Access the next state given a current state and an input bit sequence.
   *  \param currentState Current trellis state from where to search
   *  \param  input Input bit sequence specifying the branch to search
   *  \return Next state
   */
  inline BitField<size_t> getNextState(size_t currentState, size_t input) const {return nextState_[currentState * inputCount() + input];}
  /**
   *  Access the output symbol sequence given a current state and an input bit sequence.
   *  \param currentState Current trellis state from where to search
   *  \param  input Input bit sequence specifying the branch to search
   *  \return Output symbol sequence
   */
  inline BitField<size_t> getOutput(size_t currentState, size_t input) const {return output_[currentState * inputCount() + input];}

  /**
   *  Access a random access input iterator pointing to the first element in the lookup state table.
   *  This allow to access the next state given the current state and the input sequence.
   *  The table is in row major form, where each row is associated with one state
   *  and each column is associated to one input sequence (in increasing order).
   *  \return Random access input iterator to the state table
   */
  inline std::vector<BitField<size_t> >::const_iterator beginState() const {return nextState_.begin();}
  /**
   *  Access a random access input iterator referring to the past-the-end element in the lookup in the state table.
   *  This allow to access the next state given the current state and the input sequence.
   *  The table is in row major form, where each row is associated with one state
   *  and each column is associated to one input sequence (in increasing order).
   *  \return Random access input iterator to the state table
   */
  inline std::vector<BitField<size_t> >::const_iterator endState() const {return nextState_.end();}
  
  /**
   *  Access a random access input iterator pointing to the first element in the lookup output table.
   *  This allow to access the output symbol sequence of a branch given the current state and the input sequence.
   *  The table is in row major form, where each row is associated with one state
   *  and each column is associated to one input sequence (in increasing order).
   *  \return Random access input iterator to the state table
   */
  inline std::vector<BitField<size_t> >::const_iterator beginOutput() const {return output_.begin();}
  /**
   *  Access a random access input iterator referring to the past-the-end element in the lookup output table.
   *  This allow to access the output symbol sequence of a branch given the current state and the input sequence.
   *  The table is in row major form, where each row is associated with one state
   *  and each column is associated to one input sequence (in increasing order).
   *  \return Random access input iterator to the output table
   */
  inline std::vector<BitField<size_t> >::const_iterator endOutput() const {return output_.end();}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & ::BOOST_SERIALIZATION_NVP(stateSize_);
    ar & ::BOOST_SERIALIZATION_NVP(inputSize_);
    ar & ::BOOST_SERIALIZATION_NVP(outputSize_);
    ar & ::BOOST_SERIALIZATION_NVP(stateCount_);
    ar & ::BOOST_SERIALIZATION_NVP(inputCount_);
    ar & ::BOOST_SERIALIZATION_NVP(outputCount_);
    ar & ::BOOST_SERIALIZATION_NVP(nextState_);
    ar & ::BOOST_SERIALIZATION_NVP(output_);
  }
  
  size_t stateSize_;
  size_t inputSize_;
  size_t outputSize_;
  
  size_t stateCount_;
  size_t inputCount_;
  size_t outputCount_;
  
  std::vector<BitField<size_t> > nextState_;
  std::vector<BitField<size_t> > output_;
};

}

std::ostream& operator<<(std::ostream& os, const fec::Trellis& trellis);

#endif