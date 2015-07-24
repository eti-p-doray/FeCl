/*******************************************************************************
 *  \file TrellisStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-12
 *
 *  Definition of TrellisStructure class
 ******************************************************************************/

#include "TrellisStructure.h"

using namespace fec;

TrellisStructure::TrellisStructure(boost::container::vector<int> nextState, boost::container::vector<int> output, uint8_t inputSize, uint8_t outputSize, uint8_t stateSize)
{
  stateSize_ = stateSize;
  outputSize_ = outputSize;
  inputSize_ = inputSize;
  
  stateCount_ = 1<<stateSize_;
  outputCount_ = 1<<outputSize_;
  inputCount_ = 1<<inputSize_;
  
  tableSize_ = stateCount() * inputCount();
  
  nextState_.resize(inputCount()*stateCount());
  output_.resize(inputCount()*stateCount());
  
  for (int i = 0; i < stateCount(); i++) {
    for (int j = 0; j < inputCount(); j++) {
      nextState_[i*inputCount()+j] = nextState[i+j*stateCount()];
      assert(nextState_[i*inputCount()+j] < stateCount());
      output_[i*inputCount()+j] = 0;
      for (int k = 0; k < this->outputSize(); k++) {
        output_[i*inputCount()+j][k] = BitField<int>(output[i+j*stateCount()])[this->outputSize()-k-1];
      }
      assert(output_[i*inputCount()+j] < outputCount());
    }
  }
}

TrellisStructure::TrellisStructure(boost::container::vector<BitField<uint16_t> > constraintLengths, boost::container::vector<boost::container::vector<BitField<uint16_t> > > generator)
{
  inputSize_ = int(generator.size());
  outputSize_ = int(generator[0].size());
  assert(constraintLengths.size() == generator.size());
  for (int i = 0; i < inputSize(); i++) {
    assert(outputSize_ == generator[i].size());
  }
  
  outputCount_ = 1<<outputSize_;
  inputCount_ = 1<<inputSize_;

  
  boost::container::vector<BitField<uint16_t> > inputStates(constraintLengths.size(), 0);
  
  stateSize_ = 0;
  for (int i = 0; i < constraintLengths.size(); i++) {
    stateSize_ += constraintLengths[i] - 1;
  }
  stateCount_ = 1<<stateSize_;
  nextState_.resize(stateCount()*inputCount());
  output_.resize(stateCount()*inputCount());
  tableSize_ = stateCount() * inputCount();
  
  for (BitField<uint16_t> state = 0; state < stateCount(); state++) {
    for (BitField<uint16_t> input = 0; input < inputCount(); input++) {
      nextState_[state*inputCount()+input] = 0;
      BitField<uint16_t> output = 0;
      int j = 0;
      for (int i = 0; i < constraintLengths.size(); i++, j += constraintLengths[i]-1) {
        for (int k = 0; k < constraintLengths[i]-1; k++) {
          assert(i < inputStates.size());
          inputStates[i][k] = state[j+k];
        }
        
        for (int k = 0; k < outputSize_; k++) {
          assert((generator[i][k]>>1) <= stateCount());
          output[k] ^= input[i] & generator[i][k][stateSize()] ^ parity(inputStates[i] & generator[i][k]);
        }
        
        assert(state*inputCount()+input < nextState_.size());
        output_[state*inputCount()+input] ^= output;
        
        nextState_[state*inputCount()+input][j+constraintLengths[i]-2] = input[i];
        nextState_[state*inputCount()+input] |= (inputStates[i] >> 1) << j ;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const TrellisStructure& trellis)
{
  for (BitField<uint16_t> i = 0; i < trellis.stateCount(); i++) {
    for (BitField<uint16_t> j = 0; j < trellis.inputCount(); j++) {
      std::cout << i << ", " << j << ", " << trellis.getNextState(i, j) << ", " << trellis.getOutput(i, j) << std::endl;
    }
  }
  std::cout << std:: endl;
  return os;
}