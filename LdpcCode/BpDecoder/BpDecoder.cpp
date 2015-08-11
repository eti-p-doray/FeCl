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
 
 Declaration of BpDecoder abstract class
 ******************************************************************************/

#include "BpDecoder.h"
#include "TrueBp.h"
#include "MinSumBp.h"
#include "BpDecoderImpl.h"

using namespace fec;

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

/*void BpDecoder::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    parityAppDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.parityCheck().size();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.parityCheck().size();
  }
}*/

void BpDecoder::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    appDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.msgSize();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.msgSize();
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

void BpDecoder::appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  if (codeStructure().iterationCount() == 0) {
    std::copy(extrinsicIn, extrinsicIn+codeStructure_.parityCheck().size(), extrinsicOut);
    std::copy(parityIn, parityIn+codeStructure().msgSize(), messageOut);
    return;
  }
  
  std::copy(extrinsicIn, extrinsicIn+checkMetrics_.size(), checkMetrics_.begin());
  bitUpdate(parityIn);
  
  for (size_t i = 0; i < codeStructure().iterationCount() - 1; ++i) {
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
  auto check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      if (*checkBit < codeStructure().msgSize()) {
        messageOut[*checkBit] += *checkMetric;
      }
    }
  }
  
  std::copy(checkMetrics_.begin(), checkMetrics_.end(), extrinsicOut);
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
