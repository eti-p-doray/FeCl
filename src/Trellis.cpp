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
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
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
Trellis::Trellis(const std::vector<BitField<size_t>>& nextState, const std::vector<BitField<size_t>>& output, size_t inputWidth, size_t outputWidth, size_t stateWidth)
{
  stateWidth_ = stateWidth;
  outputWidth_ = outputWidth;
  inputWidth_ = inputWidth;
  
  stateCount_ = 1<<stateWidth_;
  outputCount_ = 1<<outputWidth_;
  inputCount_ = 1<<inputWidth_;
  
  nextState_.resize(inputCount()*stateCount());
  output_.resize(inputCount()*stateCount());
  
  for (size_t i = 0; i < stateCount(); i++) {
    for (BitField<size_t> j = 0; j < inputCount(); j++) {
      BitField<size_t> input = 0;
      for (int k = 0; k < this->inputWidth(); ++k) {
        input[k] = j[this->inputWidth()-k-1];
      }
      nextState_[i*inputCount()+j] = nextState[i+input*stateCount()];
      assert(nextState_[i*inputCount()+j] < stateCount());
      output_[i*inputCount()+j] = 0;
      for (size_t k = 0; k < this->outputWidth(); k++) {
        output_[i*inputCount()+j][k] = output[i+input*stateCount()].test(this->outputWidth()-k-1);
      }
      assert(output_[i*inputCount()+j] < outputCount());
    }
  }
}

Trellis::Trellis(Options options)
{
  if (options.constraintLengths_.size() != options.generator_.size()) {
    throw std::invalid_argument("Invalid number of generators");
  }
  if (options.constraintLengths_.size() != options.feedback_.size() && options.feedback_.size() != 0) {
    throw std::invalid_argument("Invalid number of feedback");
  }
  if (options.feedback_.size() == 0) {
    options.feedback_.resize(options.constraintLengths_.size());
    for (int i = 0; i < options.feedback_.size(); ++i) {
      options.feedback_[i] = 0;
      options.feedback_[i].set(options.constraintLengths_[i]-1);
    }
  }
  if (options.width_.size() == 1) {
    options.width_.resize(options.constraintLengths_.size(), options.width_[0]);
  } else if (options.width_.size() != options.constraintLengths_.size()) {
    throw std::invalid_argument("Invalid number of widths");
  }
  
  inputWidth_ = 0;
  outputWidth_ = 0;
  std::vector<BitField<size_t>> outputWidths(options.generator_[0].size(), {});
  
  for (size_t i = 0; i < options.width_.size(); ++i) {
    inputWidth_ += options.width_[i];
    for (size_t j = 0; j < outputWidths.size(); j++) {
      for (size_t k = 0; k < options.constraintLengths_[i]-1; ++k) {
        if (options.generator_[i][j].test(k)) {
          if (outputWidths[j] != 0 && outputWidths[j] != options.width_[i]) {
            throw std::invalid_argument("Invalid connection in generator");
          } else {
            outputWidths[j] = options.width_[i];
          }
        }
      }
    }
  }
  for (auto i : outputWidths) {
    outputWidth_ += i;
  }
  
  outputCount_ = 1<<outputWidth_;
  inputCount_ = 1<<inputWidth_;
  
  for (size_t i = 0; i < options.generator_.size(); i++) {
    if (outputWidths.size() != options.generator_[i].size()) {
      throw std::invalid_argument("Invalid number of generators");
    }
  }
  stateWidth_ = 0;
  for (size_t i = 0; i < options.constraintLengths_.size(); i++) {
    stateWidth_ += (options.constraintLengths_[i] - 1) * options.width_[i];
  }
  stateCount_ = 1<<stateWidth_;
  nextState_.resize(stateCount()*inputCount());
  output_.resize(stateCount()*inputCount());
  
  for (BitField<size_t> state = 0; state < stateCount(); state++) {
    for (BitField<size_t> input = 0; input < inputCount(); input++) {
      nextState_[state*inputCount()+input] = 0;
      output_[state*inputCount()+input] = 0;
      size_t stateIdx = 0;
      size_t inIdx = 0;
      std::vector<BitField<size_t>> out(outputWidths.size(), {});
      BitField<size_t> nextState = 0;
      for (size_t i = 0; i < options.constraintLengths_.size(); ++i) {
        std::vector<BitField<size_t>> s(options.constraintLengths_[i]-1, {});
        for (size_t j = 0; j < options.constraintLengths_[i]-1; ++j) {
          s[j] = state.test(stateIdx+j*options.width_[i], options.width_[i]);
        }
        BitField<size_t> in = input.test(inIdx, options.width_[i]);
        in += accumulate(options.feedback_[i], s.begin(), options.constraintLengths_[i]-1);
        in %= 1<<options.width_[i];
        
        for (size_t j = 0; j < outputWidths.size(); j++) {
          if ((options.generator_[i][j]>>options.constraintLengths_[i]) != 0) {
            throw std::invalid_argument("Invalid connection in generator");
          }
          if (options.generator_[i][j].test(options.constraintLengths_[i]-1)) {
            out[j] += in;
          }
          out[j] += accumulate(options.generator_[i][j], s.begin(), options.constraintLengths_[i]-1);
          out[j] %= 1<<options.width_[i];
        }
        if (options.feedback_[i].test(options.constraintLengths_[i]-1) != 1) {
          throw std::invalid_argument("Feedback must connect systematic");
        }
        
        size_t j;
        for (j = 0; j < options.constraintLengths_[i]-2; ++j) {
          nextState.set(stateIdx+j*options.width_[i], s[j+1], options.width_[i]);
        }
        nextState.set(stateIdx+j*options.width_[i], in, options.width_[i]);

        stateIdx += options.constraintLengths_[i]-1;
        inIdx += options.width_[i];
      }
      nextState_[state*inputCount()+input] = nextState;
      size_t outIdx = 0;
      for (size_t k = 0; k < outputWidths.size(); k++) {
        output_[state*inputCount()+input].set(outIdx, out[k], outputWidths[k]);
        outIdx += outputWidths[k];
      }
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
Trellis::Trellis(const std::vector<size_t>& constraintLengths, const std::vector<std::vector<BitField<size_t>>>& generator, std::vector<BitField<size_t>> feedback) : Trellis(Options(constraintLengths, generator).feedback(feedback))
{
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