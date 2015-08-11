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
 
 Definition of TrellisStructure class
 ******************************************************************************/

#include "TrellisStructure.h"

using namespace fec;

/**
 *  Trellis structure constructor.
 *  Construct a trellis object given state and output lookup table.
 *  Access the state size (register count) of the trellis.
 *  \param  nextState Next state lookup table given in the matlab trellis form.
 *  \param  output  Output sequence lookup table given in the matlab trellis form.
 *  \param  inputSize Number of input bits for each branch
 *  \param  outputSize  Number of output symbol for each branch
 *  \param  stateSize Number of bits (registers) in the state
 */
TrellisStructure::TrellisStructure(std::vector<size_t> nextState, std::vector<size_t> output, size_t inputSize, size_t outputSize, size_t stateSize)
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
    for (size_t j = 0; j < inputCount(); j++) {
      nextState_[i*inputCount()+j] = nextState[i+j*stateCount()];
      assert(nextState_[i*inputCount()+j] < stateCount());
      output_[i*inputCount()+j] = 0;
      for (size_t k = 0; k < this->outputSize(); k++) {
        output_[i*inputCount()+j][k] = BitField<size_t>(output[i+j*stateCount()])[this->outputSize()-k-1];
      }
      assert(output_[i*inputCount()+j] < outputCount());
    }
  }
}

/**
 *  Trellis structure constructor.
 *  Generate the state and output table lookup from a given constraint lenght and
 *  a code generator given in Proakis integer form.
 *  \param  constraintLengths Vector specifying the delay for each input bit stream.
 *  \param  output  generator Vector specifying the generator polynomials 
 *    (connections from each register to each output)
 *    associated with each input bit stream
 */
TrellisStructure::TrellisStructure(std::vector<BitField<size_t> > constraintLengths, std::vector<std::vector<BitField<size_t> > > generator)
{
  inputSize_ = size_t(generator.size());
  outputSize_ = size_t(generator[0].size());
  assert(constraintLengths.size() == generator.size());
  for (size_t i = 0; i < inputSize(); i++) {
    assert(outputSize_ == generator[i].size());
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
      BitField<size_t> output = 0;
      size_t j = 0;
      for (size_t i = 0; i < constraintLengths.size(); i++) {
        for (size_t k = 0; k < constraintLengths[i]-1; k++) {
          assert(i < inputStates.size());
          inputStates[i][k] = state[j+k];
        }
        
        for (size_t k = 0; k < outputSize_; k++) {
          assert((generator[i][k]>>1) <= stateCount());
          output[k] ^= input[i] & generator[i][k][stateSize()] ^ parity(inputStates[i] & generator[i][k]);
        }
        
        assert(state*inputCount()+input < nextState_.size());
        output_[state*inputCount()+input] ^= output;
        
        nextState_[state*inputCount()+input][j+constraintLengths[i]-2] = input[i];
        nextState_[state*inputCount()+input] |= (inputStates[i] >> 1) << j ;
        
        j += constraintLengths[i]-1;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const TrellisStructure& trellis)
{
  for (BitField<size_t> i = 0; i < trellis.stateCount(); i++) {
    for (BitField<size_t> j = 0; j < trellis.inputCount(); j++) {
      std::cout << i << ", " << j << ", " << trellis.getNextState(i, j) << ", " << trellis.getOutput(i, j) << std::endl;
    }
  }
  std::cout << std:: endl;
  return os;
}