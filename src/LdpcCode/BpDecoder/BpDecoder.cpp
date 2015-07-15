/*******************************************************************************
 *  \file BpDecoder.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-16
 *
 *  Declaration of BpDecoder abstract class
 ******************************************************************************/

#include "BpDecoder.h"
#include "TrueBp.h"
#include "MinSumBp.h"
#include "BpDecoderImpl.h"

std::unique_ptr<BpDecoder> BpDecoder::create(const LdpcCodeStructure& codeStructure)
{
  switch (codeStructure.decoderType()) {
    default:
    case LdpcCodeStructure::MinSumBp:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<MinSumBp>(codeStructure));
      break;
      
    case LdpcCodeStructure::TrueBp:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<TrueBp>(codeStructure));
      break;
  }
}

void BpDecoder::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    parityAppDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.parityCheck().size();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.parityCheck().size();
  }
}

void BpDecoder::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    appDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.parityCheck().size();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.parityCheck().size();
  }
}

void BpDecoder::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    softOutDecodeBloc(parityIn, messageOut);
    parityIn += codeStructure_.paritySize();
    messageOut += codeStructure_.msgSize();
  }
}

void BpDecoder::parityAppDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  if (codeStructure().iterationCount() == 0) {
    std::copy(extrinsicIn, extrinsicIn+codeStructure_.parityCheck().size(), extrinsicOut);
    std::copy(parityIn, parityIn+codeStructure().msgSize(), messageOut);
    return;
  }
  
  std::copy(extrinsicIn, extrinsicIn+codeStructure_.parityCheck().size(), checkMetrics_.begin());
  bitUpdate(parityIn);
  
  for (size_t i = 0; i < codeStructure().iterationCount(); ++i) {
    for (size_t j = 0; j < codeStructure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (codeStructure().syndromeCheck(hardParity_.begin())) {
      break;
    }
    
    checkUpdate();
    bitUpdate(parityIn);
  }
  
  std::copy(checkMetrics_.begin(), checkMetrics_.end(), extrinsicOut);
  std::copy(bitMetrics_.begin(), bitMetrics_.begin()+codeStructure_.msgSize(), messageOut);
}

void BpDecoder::appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  if (codeStructure().iterationCount() == 0) {
    std::copy(extrinsicIn, extrinsicIn+codeStructure().msgSize(), extrinsicOut);
    std::copy(parityIn, parityIn+codeStructure().msgSize(), messageOut);
    return;
  }
  
  auto check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      *checkMetric = parityIn[*checkBit];
      if (*checkBit < codeStructure_.msgSize()) {
        *checkMetric += extrinsicIn[*checkBit];
      }
    }
  }
  
  for (int64_t i = 0; i < codeStructure().iterationCount(); ++i) {
    for (size_t j = 0; i < codeStructure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (codeStructure().syndromeCheck(hardParity_.begin())) {
      break;
    }
    
    checkUpdate();
    bitUpdate(parityIn);
  }
  
  std::copy(bitMetrics_.begin(), bitMetrics_.begin()+codeStructure_.msgSize(), messageOut);
  std::copy(bitMetrics_.begin(), bitMetrics_.begin()+codeStructure_.msgSize(), extrinsicOut);
  for (size_t i = 0; i < codeStructure_.msgSize(); ++i) {
    extrinsicOut[i] -= extrinsicIn[i];
  }

}

void BpDecoder::softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut)
{
  if (codeStructure().iterationCount() == 0) {
    std::copy(parityIn, parityIn+codeStructure().msgSize(), messageOut);
    return;
  }
  
  auto check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      *checkMetric = parityIn[*checkBit];
    }
  }
  
  for (int64_t i = 0; i < codeStructure().iterationCount() - 1; ++i) {
    checkUpdate();
    bitUpdate(parityIn);
    
    for (size_t j = 0; j < codeStructure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (codeStructure().syndromeCheck(hardParity_.begin())) {
      break;
    }
  }
  checkUpdate();
  
  std::copy(parityIn, parityIn+codeStructure().msgSize(), messageOut);
  check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      if (*checkBit < codeStructure().msgSize()) {
        messageOut[*checkBit] += *checkMetric;
      }
    }
  }
}

BpDecoder::BpDecoder(const LdpcCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  hardParity_.resize(codeStructure_.paritySize());
  
  checkMetrics_.resize(codeStructure_.parityCheck().size());
  checkMetricsBuffer_.resize(codeStructure_.parityCheck().size());
  bitMetrics_.resize(codeStructure_.paritySize());

}
