/*******************************************************************************
 *  \file MapDecoder.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "ViterbiDecoder.h"

using namespace fec;

void ViterbiDecoder::decodeNBloc(std::vector<LlrType>::const_iterator codeIn, std::vector<uint8_t>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    decodeBloc(codeIn, messageOut);
    codeIn += codeStructure_.trellis().outputSize() * (codeStructure_.blocSize() + codeStructure_.tailSize());
    messageOut += codeStructure_.trellis().inputSize() * codeStructure_.blocSize();
  }
}

void ViterbiDecoder::decodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut)
{
  previousPathMetrics[0] = 0;
  std::fill(previousPathMetrics.begin()+1, previousPathMetrics.end(), -MAX_LLR);
  auto stateTraceBackIt = stateTraceBack.begin();
  auto inputTraceBackIt = inputTraceBack.begin();
  
  for (size_t i = 0; i < codeStructure().blocSize() + codeStructure().tailSize(); ++i) {
    std::fill(nextPathMetrics.begin(), nextPathMetrics.end(), -MAX_LLR);
    
    for (BitField<size_t> j = 0; j < codeStructure().trellis().outputCount(); ++j) {
      branchMetrics[j] = codeStructure().correlationProbability(j, parityIn, codeStructure().trellis().outputSize());
    }
    parityIn += codeStructure().trellis().outputSize();
  
    auto previousPathMetricsIt = previousPathMetrics.begin();
    auto stateIt = codeStructure().trellis().beginState();
    auto outputIt = codeStructure().trellis().beginOutput();
    for (BitField<size_t> j = 0; j < codeStructure().trellis().stateCount(); ++j) {
      for (BitField<size_t> k = 0; k < codeStructure().trellis().inputCount(); ++k) {
        auto & pathMetricRef = nextPathMetrics[*stateIt];
        LlrType competitor = *previousPathMetricsIt + branchMetrics[*outputIt];
        if (competitor >= pathMetricRef) {
          stateTraceBackIt[*stateIt] = j;
          inputTraceBackIt[*stateIt] = k;
          pathMetricRef = competitor;
        }
        stateIt++;
        outputIt++;
      }
      previousPathMetricsIt++;
    }
    stateTraceBackIt += codeStructure().trellis().stateCount();
    inputTraceBackIt += codeStructure().trellis().stateCount();
    
    LlrType max = -MAX_LLR;
    for (auto nextPathMetricsIt = nextPathMetrics.begin(); nextPathMetricsIt < nextPathMetrics.end(); nextPathMetricsIt++) {
      if (*nextPathMetricsIt > max) {
        max = *nextPathMetricsIt;
      }
    }
    if (max < -100000) {
      for (auto nextPathMetricsIt = nextPathMetrics.begin(); nextPathMetricsIt < nextPathMetrics.end(); nextPathMetricsIt++) {
        *nextPathMetricsIt -= max;
      }
    }
    swap(previousPathMetrics, nextPathMetrics);
  }
  
  stateTraceBackIt -= codeStructure().trellis().stateCount();
  inputTraceBackIt -= codeStructure().trellis().stateCount();
  
  BitField<size_t> bestState = 0;
  switch (codeStructure().endType()) {
    case ConvolutionalCodeStructure::Truncation:
      for (BitField<size_t> i = 0; i < codeStructure().trellis().stateCount(); ++i) {
        if (previousPathMetrics[i] > previousPathMetrics[bestState]) {
          bestState = i;
        }
      }
      break;
      
    default:
    case ConvolutionalCodeStructure::ZeroTail:
      break;
  }
  
  messageOut += (codeStructure().blocSize() - 1) * codeStructure().trellis().inputSize();
  for (int64_t i = codeStructure().blocSize() + codeStructure().tailSize() - 1; i >= 0; --i) {
    if (i < codeStructure().blocSize()) {
      for (BitField<size_t> j = 0; j < codeStructure().trellis().inputSize(); ++j) {
        messageOut[j] = inputTraceBackIt[bestState].test(j);
      }
      messageOut -= codeStructure().trellis().inputSize();
    }
    bestState = stateTraceBackIt[bestState];
    stateTraceBackIt -= codeStructure().trellis().stateCount();
    inputTraceBackIt -= codeStructure().trellis().stateCount();
  }
}

ViterbiDecoder::ViterbiDecoder(const ConvolutionalCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  nextPathMetrics.resize(codeStructure.trellis().stateCount());
  previousPathMetrics.resize(codeStructure.trellis().stateCount());
  stateTraceBack.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  inputTraceBack.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  branchMetrics.resize(codeStructure.trellis().outputCount());
}
