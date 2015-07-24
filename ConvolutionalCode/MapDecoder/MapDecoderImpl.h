/*******************************************************************************
 *  \file LogMap.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-28
 *  \version Last update : 2015-05-28
 *
 *  Declaration of LogMapDecoder abstract class
 ******************************************************************************/

#ifndef MAP_DECODER_IMPL_H
#define MAP_DECODER_IMPL_H

#include <algorithm>
#include <math.h>

#include "MapDecoder.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map decoder implementation.
 *  The reason for this class is to offer an common interface of map decoders 
 *  while allowing the compiler to inline implementation specific functions 
 *  by using templates instead of polymorphism.
 ******************************************************************************/
template <typename A>
class MapDecoderImpl : public MapDecoder {
public:
  MapDecoderImpl(const ConvolutionalCodeStructure& codeStructure);
  ~MapDecoderImpl() = default;
  
protected:
  virtual void appBranchMetrics(boost::container::vector<LlrType>::const_iterator code, boost::container::vector<LlrType>::const_iterator extrinsic);
  virtual void parityAppBranchMetrics(boost::container::vector<LlrType>::const_iterator code, boost::container::vector<LlrType>::const_iterator extrinsic);
  virtual void branchMetrics(boost::container::vector<LlrType>::const_iterator code);
  virtual void forwardMetrics();
  virtual void backwardMetrics();
  
  virtual void parityAPosteriori(boost::container::vector<LlrType>::iterator parityOut);
  virtual void messageAPosteriori(boost::container::vector<LlrType>::iterator messageOut);
};

template <typename A>
MapDecoderImpl<A>::MapDecoderImpl(const ConvolutionalCodeStructure& codeStructure) : MapDecoder(codeStructure)
{
  
}

template <typename A>
void MapDecoderImpl<A>::parityAPosteriori(boost::container::vector<LlrType>::iterator parityOut)
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
                               LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[uint16_t(*stateIt)])
                               );
          }
          else {
            zeroMetric = A::logAdd(
                                zeroMetric,
                                LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[uint16_t(*stateIt)])
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
void MapDecoderImpl<A>::messageAPosteriori(boost::container::vector<LlrType>::iterator messageOut)
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
                               LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[uint16_t(*stateIt)])
                               );
        ++branchMetricIt;
        ++stateIt;
        oneMetric = A::logAdd(
                              oneMetric,
                              LlrType(*branchMetricIt + *forwardMetricIt + backwardMetricIt[uint16_t(*stateIt)])
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
void MapDecoderImpl<A>::appBranchMetrics(boost::container::vector<LlrType>::const_iterator parity, boost::container::vector<LlrType>::const_iterator extrinsic)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize());
    }
    if (i < codeStructure().blocSize()) {
      for (BitField<uint16_t> j = 0; j < codeStructure().trellis().inputCount(); ++j) {
        branchInputMetrics_[j] = codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().inputSize());
      }
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (auto k : branchInputMetrics_) {
        *branchMetricIt = branchOutputMetrics_[uint16_t(*outputIt)];
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
void MapDecoderImpl<A>::parityAppBranchMetrics(boost::container::vector<LlrType>::const_iterator parity, boost::container::vector<LlrType>::const_iterator extrinsic)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize()) +
      codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().outputSize());
    }
    if (i < codeStructure().blocSize()) {
      for (BitField<uint16_t> j = 0; j < codeStructure().trellis().inputCount(); ++j) {
        branchInputMetrics_[j] = codeStructure().correlationProbability(j, extrinsic, codeStructure().trellis().inputSize());
      }
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (auto k : branchInputMetrics_) {
        *branchMetricIt = branchOutputMetrics_[uint16_t(*outputIt)];
        
        ++branchMetricIt;
        ++outputIt;
      }
    }
    parity += codeStructure().trellis().outputSize();
    extrinsic += codeStructure().trellis().inputSize();
  }
}

template <typename A>
void MapDecoderImpl<A>::branchMetrics(boost::container::vector<LlrType>::const_iterator parity)
{
  int i = 0;
  for (auto branchMetricIt = branchMetrics_.begin(); branchMetricIt < branchMetrics_.end(); ++i) {
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchOutputMetrics_[j] = codeStructure().correlationProbability(j, parity, codeStructure().trellis().outputSize());
    }
    
    for (auto outputIt = codeStructure().trellis().beginOutput(); outputIt < codeStructure().trellis().endOutput();) {
      for (auto k : branchInputMetrics_) {
        *branchMetricIt = branchOutputMetrics_[uint16_t(*outputIt)];
        
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
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<uint16_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & forwardMetricRef = forwardMetricIt[codeStructure().trellis().stateCount() + uint16_t(*stateIt)];
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
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      if (forwardMetricIt[j] > max) {
        max = forwardMetricIt[j];
      }
    }
    if (max < -THRESHOLD_LLR) {
      for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
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
    case ConvolutionalCodeStructure::ZeroTail:
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
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<uint16_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & backwardMetricRef = backwardMetricIt[j];
        backwardMetricRef =
        A::logAdd(
               backwardMetricRef,
               LlrType(backwardMetricIt[uint16_t(*stateIt)+codeStructure().trellis().stateCount()] + *branchMetricIt)
               );
        ++branchMetricIt;
        ++stateIt;
      }
    }
    branchMetricIt -= 2*codeStructure().trellis().tableSize();
    for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      if (backwardMetricIt[j] > max) {
        max = backwardMetricIt[j];
      }
    }
    if (max < -THRESHOLD_LLR) {
      for (BitField<uint16_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
        backwardMetricIt[j] -= max;
      }
    }
  }
}
  
}

#endif
