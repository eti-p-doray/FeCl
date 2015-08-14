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
  MapDecoderImpl(const ConvolutionalCodeStructure& codeStructure);
  ~MapDecoderImpl() = default;
  
protected:
  virtual void appBranchMetrics(std::vector<LlrType>::const_iterator code, std::vector<LlrType>::const_iterator extrinsic);
  virtual void parityAppBranchMetrics(std::vector<LlrType>::const_iterator code, std::vector<LlrType>::const_iterator extrinsic);
  virtual void branchMetrics(std::vector<LlrType>::const_iterator code);
  virtual void forwardMetrics();
  virtual void backwardMetrics();
  
  virtual void parityAPosteriori(std::vector<LlrType>::iterator parityOut);
  virtual void messageAPosteriori(std::vector<LlrType>::iterator parityOut);
  virtual void messageExtrinsic(std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinscOut);
};

template <typename A>
MapDecoderImpl<A>::MapDecoderImpl(const ConvolutionalCodeStructure& codeStructure) : MapDecoder(codeStructure)
{
  
}

template <typename A>
void MapDecoderImpl<A>::parityAPosteriori(std::vector<LlrType>::iterator parityOut)
{
  auto backwardMetricIt = backwardMetrics_.begin();
  
  for (size_t i = 0; i < codeStructure().blocSize()+codeStructure().tailSize(); ++i) {
    for (size_t j = 0; j < codeStructure().trellis().outputSize(); ++j) {
      auto branchMetricIt = branchMetrics_.begin() + i * codeStructure().trellis().tableSize();
      auto forwardMetricIt = forwardMetrics_.begin() + i * codeStructure().trellis().stateCount();
      
      LlrType oneMetric = -MAX_LLR;
      LlrType zeroMetric = -MAX_LLR;
      
      auto outputIt = codeStructure().trellis().beginOutput();
      for (auto stateIt = codeStructure().trellis().beginState(); stateIt < codeStructure().trellis().endState();) {
        for (int l = 0; l < 2; ++l) {
          if (outputIt->test(j)) {
            oneMetric = A::logAdd(
                               oneMetric,
                               LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                               );
          }
          else {
            zeroMetric = A::logAdd(
                                zeroMetric,
                                LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                                );
          }
          branchMetricIt++;
          stateIt++;
          outputIt++;
        }
        forwardMetricIt++;
      }
      *parityOut = oneMetric - zeroMetric;
      parityOut++;
    }
    backwardMetricIt += codeStructure().trellis().stateCount();
  }
}

template <typename A>
void MapDecoderImpl<A>::messageExtrinsic(std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  auto backwardMetricIt = backwardMetrics_.begin();
  
  for (size_t i = 0; i < codeStructure().blocSize() + codeStructure().tailSize(); ++i) {
    for (size_t j = 0; j < codeStructure().trellis().inputSize(); ++j) {
      auto branchMetricIt = branchMetrics_.cbegin() + i * codeStructure().trellis().tableSize();
      auto forwardMetricIt = forwardMetrics_.cbegin() + i * codeStructure().trellis().stateCount();
      
      LlrType oneMetric = -MAX_LLR;
      LlrType zeroMetric = -MAX_LLR;
      
      for (auto stateIt = codeStructure().trellis().beginState(); stateIt < codeStructure().trellis().endState();) {
        zeroMetric = A::logAdd(
                               zeroMetric,
                               LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                               );
        ++branchMetricIt;
        ++stateIt;
        oneMetric = A::logAdd(
                              oneMetric,
                              LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                              );
        ++branchMetricIt;
        ++stateIt;
        ++forwardMetricIt;
      }
      if (i < codeStructure().blocSize()) {
        *messageOut = oneMetric - zeroMetric;
      }
      *extrinsicOut = oneMetric - zeroMetric - *extrinsicIn;
      ++extrinsicOut;
      ++extrinsicIn;
      ++messageOut;
    }
    backwardMetricIt += codeStructure().trellis().stateCount();
  }
}
  
template <typename A>
void MapDecoderImpl<A>::messageAPosteriori(std::vector<LlrType>::iterator messageOut)
{
  auto backwardMetricIt = backwardMetrics_.begin();
  
  for (size_t i = 0; i < codeStructure().blocSize(); ++i) {
    for (size_t j = 0; j < codeStructure().trellis().inputSize(); ++j) {
      auto branchMetricIt = branchMetrics_.cbegin() + i * codeStructure().trellis().tableSize();
      auto forwardMetricIt = forwardMetrics_.cbegin() + i * codeStructure().trellis().stateCount();
      
      LlrType oneMetric = -MAX_LLR;
      LlrType zeroMetric = -MAX_LLR;
      
      for (auto stateIt = codeStructure().trellis().beginState(); stateIt < codeStructure().trellis().endState();) {
        zeroMetric = A::logAdd(
                               zeroMetric,
                               LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                               );
        ++branchMetricIt;
        ++stateIt;
        oneMetric = A::logAdd(
                              oneMetric,
                              LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[size_t(*stateIt)])
                              );
        ++branchMetricIt;
        ++stateIt;
        ++forwardMetricIt;
      }
      *messageOut = oneMetric - zeroMetric;
      ++messageOut;
    }
    backwardMetricIt += codeStructure().trellis().stateCount();
  }
}

template <typename A>
void MapDecoderImpl<A>::appBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize());
    }
    if (i < codeStructure().blocSize() + codeStructure().tailSize()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().inputCount(); ++j) {
        branchInputMetrics_[j] = codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().inputSize());
      }
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (auto k : branchInputMetrics_) {
        *branchMetricIt = branchOutputMetrics_[size_t(*outputIt)];
        *branchMetricIt += k;
        
        ++branchMetricIt;
        ++outputIt;
      }
    }
    parity += codeStructure().trellis().outputSize();
    extrinsic += codeStructure().trellis().inputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::parityAppBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize()) +
      codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().outputSize());
    }
    if (i < codeStructure().blocSize()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().inputCount(); ++j) {
        branchInputMetrics_[j] = codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().inputSize());
      }
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (size_t k = 0; k < branchInputMetrics_.size(); ++k) {
        *branchMetricIt = branchOutputMetrics_[size_t(*outputIt)];
        
        ++branchMetricIt;
        ++outputIt;
      }
    }
    parity += codeStructure().trellis().outputSize();
    extrinsic += codeStructure().trellis().inputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::branchMetrics(std::vector<LlrType>::const_iterator parity)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize());
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (size_t k = 0; k < branchInputMetrics_.size(); ++k) {
        *branchMetricIt = branchOutputMetrics_[size_t(*outputIt)];
        
        ++branchMetricIt;
        ++outputIt;
      }
    }
    parity += codeStructure().trellis().outputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::forwardMetrics()
{
  auto forwardMetricIt = forwardMetrics_.begin();
  auto branchMetricIt = branchMetrics_.cbegin();
  
  *forwardMetricIt = 0;
  std::fill(forwardMetricIt+1, forwardMetricIt + codeStructure().trellis().stateCount(), -MAX_LLR);
  
  for (; forwardMetricIt < forwardMetrics_.end() - codeStructure().trellis().stateCount();) {
    std::fill(forwardMetricIt + codeStructure().trellis().stateCount(), forwardMetricIt + 2*codeStructure().trellis().stateCount(), -MAX_LLR);
    auto stateIt = codeStructure().trellis().beginState();
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & forwardMetricRef = forwardMetricIt[codeStructure().trellis().stateCount() + size_t(*stateIt)];
        forwardMetricRef =
        A::logAdd(
               forwardMetricRef,
               LlrType(forwardMetricIt[j] + *branchMetricIt)
               );
        ++branchMetricIt;
        ++stateIt;
      }
    }
    forwardMetricIt += codeStructure().trellis().stateCount();
    LlrType max = -MAX_LLR;
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      if (forwardMetricIt[j] > max) {
        max = forwardMetricIt[j];
      }
    }
    if (max < -THRESHOLD_LLR) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
        forwardMetricIt[j] -= max;
      }
    }
  }
}

template <typename A>
void MapDecoderImpl<A>::backwardMetrics()
{
  auto backwardMetricIt = backwardMetrics_.end()-codeStructure().trellis().stateCount();
  
  switch (codeStructure().endType()) {
    case ConvolutionalCodeStructure::PaddingTail:
      *backwardMetricIt = 0;
      std::fill(backwardMetricIt+1, backwardMetricIt + codeStructure().trellis().stateCount(), -MAX_LLR);
      backwardMetricIt -= codeStructure().trellis().stateCount();
      break;
      
    default:
    case ConvolutionalCodeStructure::Truncation:
      std::fill(backwardMetricIt, backwardMetricIt + codeStructure().trellis().stateCount(), 0);
      backwardMetricIt -= codeStructure().trellis().stateCount();
      break;
  }
  
  auto branchMetricIt = branchMetrics_.cend()-codeStructure().trellis().stateCount()*codeStructure().trellis().inputCount();
  
  for ( ; backwardMetricIt >= backwardMetrics_.begin(); backwardMetricIt -= codeStructure().trellis().stateCount()) {
    std::fill(backwardMetricIt, backwardMetricIt + codeStructure().trellis().stateCount(), -MAX_LLR);
    auto stateIt = codeStructure().trellis().beginState();
    LlrType max = -MAX_LLR;
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & backwardMetricRef = backwardMetricIt[j];
        backwardMetricRef =
        A::logAdd(
               backwardMetricRef,
               LlrType(backwardMetricIt[size_t(*stateIt)+codeStructure().trellis().stateCount()] + *branchMetricIt)
               );
        ++branchMetricIt;
        ++stateIt;
      }
    }
    branchMetricIt -= 2*codeStructure().trellis().tableSize();
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      if (backwardMetricIt[j] > max) {
        max = backwardMetricIt[j];
      }
    }
    if (max < -THRESHOLD_LLR) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
        backwardMetricIt[j] -= max;
      }
    }
  }
}
  
}

#endif
