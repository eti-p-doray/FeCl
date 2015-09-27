/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "Trellis.h"

using namespace fec;

/**
 *  Trellis structure constructor.
 *  Constructs a trellis object given state and output lookup table.
 *  Access the state size (register count) of the trellis.
 *  \param  nextState Next state lookup table given in the matlab trellis form.
 *  \param  output  Output sequence lookup table given in the matlab trellis form.
 *  \param  inputSize Number of input bits for each branch
 *  \param  outputSize  Number of output symbol for each branch
 *  \param  stateSize Number of bits (registers) in the state
 */
Trellis::Trellis(std::vector<BitField<size_t>> nextState, std::vector<BitField<size_t>> output, size_t inputSize, size_t outputSize, size_t stateSize)
{
  stateSize_ = stateSize;
  outputSize_ = outputSize;
  inputSize_ = inputSize;
  
  stateCount_ = 1<<stateSize_;
  outputCount_ = 1<<outputSize_;
  inputCount_ = 1<<inputSize_;
  
  nextState_.resize(inputCount()*stateCount());
  output_.resize(inputCount()*stateCount());
  
  for (size_t i = 0; i < stateCount(); i++) {
    for (BitField<size_t> j = 0; j < inputCount(); j++) {
      BitField<size_t> input = 0;
      for (int k = 0; k < this->inputSize(); ++k) {
        input[k] = j[this->inputSize()-k-1];
      }
      nextState_[i*inputCount()+j] = nextState[i+input*stateCount()];
      assert(nextState_[i*inputCount()+j] < stateCount());
      output_[i*inputCount()+j] = 0;
      for (size_t k = 0; k < this->outputSize(); k++) {
        output_[i*inputCount()+j][k] = output[i+input*stateCount()].test(this->outputSize()-k-1);
      }
      assert(output_[i*inputCount()+j] < outputCount());
    }
  }
}

/**
 *  Trellis structure constructor.
 *  Generates the state and output table lookup from a given constraint lenght and
 *  a code generator given in Proakis integer form.
 *  \snippet Convolutional.cpp Creating a trellis
 *  \param  constraintLengths Vector specifying the delay for each input bit stream.
 *  \param  output  generator Vector specifying the generator polynomials 
 *    (connections from each register to each output)
 *    associated with each input bit stream
 */
Trellis::Trellis(std::vector<BitField<size_t>> constraintLengths, std::vector<std::vector<BitField<size_t>>> generator, std::vector<BitField<size_t>> feedback)
{
  inputSize_ = size_t(generator.size());
  outputSize_ = size_t(generator[0].size());
  if (constraintLengths.size() != generator.size()) {
    throw std::invalid_argument("Invalid number of generators");
  }
  if (constraintLengths.size() != feedback.size() && feedback.size() != 0) {
    throw std::invalid_argument("Invalid number of feedback");
  }
  for (size_t i = 0; i < inputSize(); i++) {
    if (outputSize_ != generator[i].size()) {
      throw std::invalid_argument("Invalid number of generators");
    }
  }
  if (feedback.size() == 0) {
    feedback.resize(constraintLengths.size());
    for (int i = 0; i < feedback.size(); ++i) {
      feedback[i] = 0;
      feedback[i].set(constraintLengths[i]-1);
    }
  }
  
  outputCount_ = 1<<outputSize_;
  inputCount_ = 1<<inputSize_;
  
  
  std::vector<BitField<size_t> > inputStates(constraintLengths.size(), 0);
  
  stateSize_ = 0;
  for (size_t i = 0; i < constraintLengths.size(); i++) {
    stateSize_ += constraintLengths[i] - 1;
  }
  stateCount_ = 1<<stateSize_;
  nextState_.resize(stateCount()*inputCount());
  output_.resize(stateCount()*inputCount());
  
  for (BitField<size_t> state = 0; state < stateCount(); state++) {
    for (BitField<size_t> input = 0; input < inputCount(); input++) {
      nextState_[state*inputCount()+input] = 0;
      output_[state*inputCount()+input] = 0;
      size_t j = 0;
      for (size_t i = 0; i < constraintLengths.size(); i++) {
        for (size_t k = 0; k < constraintLengths[i]-1; k++) {
          inputStates[i][k] = state[j+k];
        }
        
        for (size_t k = 0; k < outputSize_; k++) {
          if ((generator[i][k]>>constraintLengths[i]) != 0) {
            throw std::invalid_argument("Invalid connection in generator");
          }
          output_[state*inputCount()+input][k] ^=
          ((input[i] ^ parity(inputStates[i] & feedback[i])) & generator[i][k].test(constraintLengths[i]-1))
          ^ parity(inputStates[i] & generator[i][k]);
        }
        
        if (feedback[i].test(constraintLengths[i]-1) != 1) {
          throw std::invalid_argument("Feedback must connect systematic");
        }
        nextState_[state*inputCount()+input][j+constraintLengths[i]-2] = (input[i] & feedback[i].test(constraintLengths[i]-1))
        ^ parity(inputStates[i] & feedback[i]);
        nextState_[state*inputCount()+input] |= (inputStates[i] >> 1) << j ;
        
        j += constraintLengths[i]-1;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Trellis& trellis)
{
  for (BitField<size_t> i = 0; i < trellis.stateCount(); i++) {
    for (BitField<size_t> j = 0; j < trellis.inputCount(); j++) {
      std::cout << i << ", " << j << ", " << trellis.getNextState(i, j) << ", " << trellis.getOutput(i, j) << std::endl;
    }
  }
  std::cout << std:: endl;
  return os;
}