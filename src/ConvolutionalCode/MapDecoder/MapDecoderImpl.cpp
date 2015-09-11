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
 
 Declaration of LogMapDecoder abstract class
 ******************************************************************************/

#ifndef MAP_DECODER_IMPL_H
#define MAP_DECODER_IMPL_H

#include <algorithm>
#include <math.h>

#include "MapDecoder.h"

namespace fec {

/**
 *  This class contains the concrete implementation of the map decoder.
 *  This algorithm is used for decoding with a-priori information 
 *  in a ConvolutionalCode.
 *  The reason for this class is to offer an common interface of map decoders 
 *  while allowing the compiler to inline implementation specific functions 
 *  by using templates instead of polymorphism.
 */
template <typename A>
class MapDecoderImpl : public MapDecoder {
public:
  MapDecoderImpl(const ConvolutionalCode::Structure& codeStructure);
  ~MapDecoderImpl() = default;
  
protected:
  virtual void appBranchUpdate(std::vector<LlrType>::const_iterator code, std::vector<LlrType>::const_iterator extrinsic);
  virtual void branchUpdate(std::vector<LlrType>::const_iterator code);
  virtual void forwardUpdate();
  virtual void backwardUpdate();
  
  virtual void aPosterioriUpdate(std::vector<LlrType>::iterator parityOut);
  virtual void extrinsicUpdate(std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinscOut);
  
private:
  A a;
};

template <typename A>
MapDecoderImpl<A>::MapDecoderImpl(const ConvolutionalCode::Structure& codeStructure) : MapDecoder(codeStructure)
{
  
}

template <typename A>
void MapDecoderImpl<A>::extrinsicUpdate(std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  auto backwardMetric = backwardMetrics_.begin();
  for (size_t i = 0; i < codeStructure().blocSize() + codeStructure().tailSize(); ++i) {
    auto branchMetric = branchMetrics_.begin() + i * codeStructure().trellis().tableSize();
    auto forwardMetric = forwardMetrics_.cbegin() + i * codeStructure().trellis().stateCount();
    
    for (auto state = codeStructure().trellis().beginState(); state < codeStructure().trellis().endState(); ) {
      for (BitField<size_t> input = 0; input < codeStructure().trellis().inputCount(); ++input) {
        branchMetric[input] = a.f(branchMetric[input] + *forwardMetric + backwardMetric[size_t(stateIt[input])]);
      }
      state += codeStructure().trellis().inputCount();
      branchMetric += codeStructure().trellis().inputCount();
      ++forwardMetric;
    }
    
    for (size_t j = 0; j < codeStructure().trellis().inputSize(); ++j) {
      LlrType oneMetric = a.f(-MAX_LLR);
      LlrType zeroMetric = a.f(-MAX_LLR);
      auto branchMetric = branchMetrics_.cbegin() + i * codeStructure().trellis().tableSize();
      for (size_t k = 0; k < codeStructure().trellis().stateCount(); ++k) {
        for (BitField<size_t> input = 0; input < codeStructure().trellis().inputCount(); ++input) {
          if (input.test(j)) {
            oneMetric = a.step(oneMetric,branchMetric[input]);
          }
          else {
            zeroMetric = a.step(zeroMetric,branchMetric[input]);
          }
        }
        branchMetric += codeStructure().trellis().inputCount();
      }
      
      if (i < codeStructure().blocSize()) {
        messageOut[i] = a.b(oneMetric) - a.b(zeroMetric);
        extrinsicOut[i] = messageOut[i] - extrinsicIn[i];
      }
      else {
        extrinsicOut[i] = a.b(oneMetric) - a.b(zeroMetric) - extrinsicIn[i];
      }
    }
    backwardMetric += codeStructure().trellis().stateCount();
  }
}
  
template <typename A>
void MapDecoderImpl<A>::aPosterioriUpdate(std::vector<LlrType>::iterator messageOut)
{
  auto backwardMetric = backwardMetrics_.begin();
  for (size_t i = 0; i < codeStructure().blocSize(); ++i) {
    auto branchMetric = branchMetrics_.begin() + i * codeStructure().trellis().tableSize();
    auto forwardMetric = forwardMetrics_.cbegin() + i * codeStructure().trellis().stateCount();
    
    for (auto state = codeStructure().trellis().beginState(); stateIt < codeStructure().trellis().endState(); ) {
      for (BitField<size_t> input = 0; input < codeStructure().trellis().inputCount(); ++input) {
        branchMetric[input] = a.f(branchMetric[input] + *forwardMetric + backwardMetricIt[size_t(stateIt[input])]);
      }
      state += codeStructure().trellis().inputCount();
      branchMetric += codeStructure().trellis().inputCount();
      ++forwardMetric;
    }
    
    for (size_t j = 0; j < codeStructure().trellis().inputSize(); ++j) {
      LlrType oneMetric = a.f(-MAX_LLR);
      LlrType zeroMetric = a.f(-MAX_LLR);
      auto branchMetric = branchMetrics_.cbegin() + i * codeStructure().trellis().tableSize();
      for (size_t k = 0; k < codeStructure().trellis().stateCount(); ++k) {
        for (BitField<size_t> input = 0; input < codeStructure().trellis().inputCount(); ++input) {
          if (input.test(j)) {
            oneMetric = a.step(oneMetric,branchMetric[input]);
          }
          else {
            zeroMetric = a.step(zeroMetric,branchMetric[input]);
          }
        }
        branchMetric += codeStructure().trellis().inputCount();
      }
      messageOut[i] = a.b(oneMetric) - a.b(zeroMetric);
    }
    backwardMetric += codeStructure().trellis().stateCount();
  }
}
  
template <typename A>
void MapDecoderImpl<A>::appBranchUpdate(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = correlation<LlrType>(j, parity, codeStructure().trellis().outputSize());
    }
    if (i < codeStructure().blocSize() + codeStructure().tailSize()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().inputCount(); ++j) {
        branchInputMetrics_[j] = correlation<LlrType>(j, extrinsic, codeStructure().trellis().inputSize());
      }
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (size_t k = 0; k < branchInputMetrics_.size(); ++k) {
        branchMetric[k] = branchOutputMetrics_[size_t(outputIt[k])];
        branchMetric[k] += branchInputMetrics_[k];
      }
      outputIt += branchInputMetrics_.size();
      branchMetric += branchInputMetrics_.size();
    }
    
    parity += codeStructure().trellis().outputSize();
    extrinsic += codeStructure().trellis().inputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::branchUpdate(std::vector<LlrType>::const_iterator parity)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = correlation<LlrType>(j, parity, codeStructure().trellis().outputSize());
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (size_t k = 0; k < branchInputMetrics_.size(); ++k) {
        branchMetricIt[k] = branchOutputMetrics_[size_t(outputIt[k])];
      }
      outputIt += branchInputMetrics_.size();
      branchMetricIt += branchInputMetrics_.size();
    }
    parity += codeStructure().trellis().outputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::forwardUpdate()
{
  auto forwardMetricIt = forwardMetrics_.begin();
  auto branchMetricIt = branchMetrics_.cbegin();
  
  *forwardMetricIt = 0;
  std::fill(forwardMetricIt+1, forwardMetricIt + codeStructure().trellis().stateCount(), -MAX_LLR);
  
  for (; forwardMetricIt < forwardMetrics_.end() - codeStructure().trellis().stateCount();) {
    std::fill(forwardMetricIt + codeStructure().trellis().stateCount(), forwardMetricIt + 2*codeStructure().trellis().stateCount(), a.f(-MAX_LLR));
    auto stateIt = codeStructure().trellis().beginState();
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & forwardMetricRef = forwardMetricIt[codeStructure().trellis().stateCount() + size_t(stateIt[k])];
        forwardMetricRef =
        a.step(forwardMetricRef, a.f(LlrType(forwardMetricIt[j] + branchMetricIt[k])));
      }
      branchMetricIt += codeStructure().trellis().inputCount();
      stateIt += codeStructure().trellis().inputCount();
    }
    forwardMetricIt += codeStructure().trellis().stateCount();
    LlrType max = -MAX_LLR;
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      forwardMetricIt[j] = a.b(forwardMetricIt[j]);
      if (forwardMetricIt[j] > max) {
        max = forwardMetricIt[j];
      }
    }
    if (max > a.threshold()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
        forwardMetricIt[j] -= max;
      }
    }
  }
}

template <typename A>
void MapDecoderImpl<A>::backwardUpdate()
{
  auto backwardMetricIt = backwardMetrics_.end()-codeStructure().trellis().stateCount();
  
  switch (codeStructure().terminationType()) {
    case ConvolutionalCode::Tail:
      *backwardMetricIt = 0;
      std::fill(backwardMetricIt+1, backwardMetricIt + codeStructure().trellis().stateCount(), -MAX_LLR);
      backwardMetricIt -= codeStructure().trellis().stateCount();
      break;
      
    default:
    case ConvolutionalCode::Truncation:
      std::fill(backwardMetricIt, backwardMetricIt + codeStructure().trellis().stateCount(), 0.0);
      backwardMetricIt -= codeStructure().trellis().stateCount();
      break;
  }
  
  auto branchMetricIt = branchMetrics_.cend()-codeStructure().trellis().stateCount()*codeStructure().trellis().inputCount();
  
  for ( ; backwardMetricIt >= backwardMetrics_.begin(); backwardMetricIt -= codeStructure().trellis().stateCount()) {
    std::fill(backwardMetricIt, backwardMetricIt + codeStructure().trellis().stateCount(), a.f(-MAX_LLR));
    auto stateIt = codeStructure().trellis().beginState();
    LlrType max = -MAX_LLR;
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & backwardMetricRef = backwardMetricIt[j];
        backwardMetricRef =
        a.step(
               backwardMetricRef,
               a.f(LlrType(backwardMetricIt[size_t(stateIt[k])+codeStructure().trellis().stateCount()] + branchMetricIt[k]))
               );
      }
      branchMetricIt += codeStructure().trellis().inputCount();
      stateIt += codeStructure().trellis().inputCount();
    }
    branchMetricIt -= 2*codeStructure().trellis().tableSize();
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      backwardMetricIt[j] = a.b(backwardMetricIt[j]);
      if (backwardMetricIt[j] > max) {
        max = backwardMetricIt[j];
      }
    }
    if (max > a.threshold()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
        backwardMetricIt[j] -= max;
      }
    }
  }
}
  
}

#endif
