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

#include "ViterbiDecoderImpl.h"

using namespace fec;
using namespace fec::detail;

/**
 *  Decodes one bloc of information bits.
 *  \param  parityIn  Input iterator pointing to the first element
 *    in the parity L-value sequence
 *  \param  messageOut[out] Output iterator pointing to the first element
 *    in the decoded msg sequence.
 *    Output needs to be pre-allocated.
 */
template <class LlrMetrics>
void ViterbiDecoderImpl<LlrMetrics>::decodeBlock(std::vector<double>::const_iterator parityIn, std::vector<BitField<size_t>>::iterator messageOut)
{
  previousPathMetrics_[0] = 0;
  std::fill(previousPathMetrics_.begin()+1, previousPathMetrics_.end(), -llrMetrics_.max());
  auto stateTraceBack = stateTraceBack_.begin();
  auto inputTraceBack = inputTraceBack_.begin();
  
  for (size_t i = 0; i < structure().length() + structure().tailSize(); ++i) {
    std::fill(nextPathMetrics_.begin(), nextPathMetrics_.end(), -llrMetrics_.max());
    
    for (BitField<size_t> j = 0; j < structure().trellis().outputCount(); ++j) {
      branchMetrics_[j] = correlation<LlrMetrics>(j, parityIn, structure().trellis().outputSize());
    }
    parityIn += structure().trellis().outputSize();
  
    auto previousPathMetricsIt = previousPathMetrics_.begin();
    auto state = structure().trellis().beginState();
    auto output = structure().trellis().beginOutput();
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
        auto & pathMetricRef = nextPathMetrics_[state[k]];
        typename LlrMetrics::Type competitor = *previousPathMetricsIt + branchMetrics_[output[k]];
        if (competitor >= pathMetricRef) {
          stateTraceBack[state[k]] = j;
          inputTraceBack[state[k]] = k;
          pathMetricRef = competitor;
        }
      }
      state+=structure().trellis().inputCount();
      output+=structure().trellis().inputCount();
      previousPathMetricsIt++;
    }
    stateTraceBack += structure().trellis().stateCount();
    inputTraceBack += structure().trellis().stateCount();
    
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (auto nextPathMetric = nextPathMetrics_.begin(); nextPathMetric < nextPathMetrics_.end(); nextPathMetric++) {
      if (*nextPathMetric > max) {
        max = *nextPathMetric;
      }
    }
    for (auto nextPathMetric = nextPathMetrics_.begin(); nextPathMetric < nextPathMetrics_.end(); nextPathMetric++) {
      *nextPathMetric -= max;
    }
    swap(previousPathMetrics_, nextPathMetrics_);
  }
  
  stateTraceBack -= structure().trellis().stateCount();
  inputTraceBack -= structure().trellis().stateCount();
  
  BitField<size_t> bestState = 0;
  switch (structure().termination()) {
    case Trellis::Truncate:
      for (BitField<size_t> i = 0; i < structure().trellis().stateCount(); ++i) {
        if (previousPathMetrics_[i] > previousPathMetrics_[bestState]) {
          bestState = i;
        }
      }
      break;
      
    default:
    case Trellis::Tail:
      break;
  }
  
  messageOut += (structure().length() - 1) * structure().trellis().inputSize();
  for (int64_t i = structure().length() + structure().tailSize() - 1; i >= 0; --i) {
    if (i < structure().length()) {
      for (BitField<size_t> j = 0; j < structure().trellis().inputSize(); ++j) {
        messageOut[j] = inputTraceBack[bestState].test(j);
      }
      messageOut -= structure().trellis().inputSize();
    }
    bestState = stateTraceBack[bestState];
    stateTraceBack -= structure().trellis().stateCount();
    inputTraceBack -= structure().trellis().stateCount();
  }
}

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
template <class LlrMetrics>
ViterbiDecoderImpl<LlrMetrics>::ViterbiDecoderImpl(const Convolutional::Structure& structure) :
ViterbiDecoder(structure)
{
  nextPathMetrics_.resize(structure.trellis().stateCount());
  previousPathMetrics_.resize(structure.trellis().stateCount());
  stateTraceBack_.resize((structure.length()+structure.tailSize())*structure.trellis().stateCount());
  inputTraceBack_.resize((structure.length()+structure.tailSize())*structure.trellis().stateCount());
  branchMetrics_.resize(structure.trellis().outputCount());
}

template class fec::detail::ViterbiDecoderImpl<FloatLlrMetrics>;
