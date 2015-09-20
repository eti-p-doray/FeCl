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
 
 Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "ViterbiDecoderImpl.h"

using namespace fec;

/**
 *  Decodes one bloc of information bits.
 *  \param  parityIn  Input iterator pointing to the first element
 *    in the parity L-value sequence
 *  \param  messageOut[out] Output iterator pointing to the first element
 *    in the decoded msg sequence.
 *    Output needs to be pre-allocated.
 */
template <class LlrMetrics>
void ViterbiDecoderImpl<LlrMetrics>::decodeBlock(std::vector<LlrType>::const_iterator parityIn, std::vector<BitField<bool>>::iterator messageOut)
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
    case Convolutional::Truncate:
      for (BitField<size_t> i = 0; i < structure().trellis().stateCount(); ++i) {
        if (previousPathMetrics_[i] > previousPathMetrics_[bestState]) {
          bestState = i;
        }
      }
      break;
      
    default:
    case Convolutional::Tail:
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

template class fec::ViterbiDecoderImpl<FloatLlrMetrics>;
