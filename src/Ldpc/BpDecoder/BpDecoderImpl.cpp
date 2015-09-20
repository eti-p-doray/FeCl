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
 
 Declaration of BpDecoderImpl class
 ******************************************************************************/

#include "BpDecoderImpl.h"

using namespace fec;

template <class LlrMetrics, template <class> class BoxSumAlg>
BpDecoderImpl<LlrMetrics, BoxSumAlg>::BpDecoderImpl(const Ldpc::Structure& structure) :
BpDecoder(structure)
{
  hardParity_.resize(this->structure().paritySize());
  parity_.resize(this->structure().paritySize());
  checkMetrics_.resize(this->structure().checks().size());
  checkMetricsBuffer_.resize(this->structure().checks().size());
  bitMetrics_.resize(this->structure().paritySize());
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg)
{
  std::copy(parity, parity+structure().paritySize(), parity_.begin());

  if (structure().iterations() > 0) {
    for (size_t i = 0; i < structure().checks().size(); ++i) {
      checkMetrics_[i] = parity_[structure().checks().at(i)];
    }
  }
  
  bool succes = false;
  for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
    checkUpdate();
    bitUpdate();
    
    for (size_t j = 0; j < structure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      succes = true;
      break;
    }
  }
  if (!succes) {
    checkUpdate();
  }
  
  std::copy(parity_.begin(), parity_.begin() + bitMetrics_.size(), bitMetrics_.begin());
  for (size_t i = 0; i < structure().checks().size(); ++i) {
    bitMetrics_[structure().checks().at(i)] += checkMetrics_[i];
  }
  for (size_t i = 0; i < structure().msgSize(); ++i) {
    msg[i] = bitMetrics_[i] >= 0;
  }
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output)
{
  std::copy(input.parity(), input.parity()+structure().paritySize(), parity_.begin());
  if (input.hasSyst()) {
    for (size_t i = 0; i < structure().systSize(); ++i) {
      parity_[i] += input.syst()[i];
    }
  }
  if (input.hasState()) {
    std::copy(input.state(), input.state()+structure().stateSize(), checkMetrics_.begin());
  }
  
  if (structure().iterations() > 0) {
    if (input.hasState()) {
      bitUpdate();
    }
    else {
      for (size_t i = 0; i < structure().checks().size(); ++i) {
        checkMetrics_[i] = parity_[structure().checks().at(i)];
      }
    }
  }

  bool succes = false;
  for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
    checkUpdate();
    bitUpdate();
    
    for (size_t j = 0; j < structure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      succes = true;
      break;
    }
  }
  if (!succes) {
    checkUpdate();
  }
  
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  for (size_t i = 0; i < structure().checks().size(); ++i) {
    bitMetrics_[structure().checks().at(i)] += checkMetrics_[i];
  }
  
  if (output.hasSyst()) {
    std::copy(bitMetrics_.begin(), bitMetrics_.begin()+structure().systSize(), output.syst());
  }
  if (output.hasParity()) {
    std::copy(bitMetrics_.begin(), bitMetrics_.begin()+structure().paritySize(), output.parity());
  }
  if (output.hasState()) {
    std::copy(checkMetrics_.begin(), checkMetrics_.end(), output.state());
  }
  if (output.hasMsg()) {
    for (size_t i = 0; i < structure().msgSize(); ++i) {
      output.msg()[i] = parity_[i] + bitMetrics_[i];
    }
  }
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::checkUpdate()
{
  auto checkMetric = checkMetrics_.begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto check = structure().checks().begin(); check < structure().checks().end();  ++check) {
    auto first = checkMetric;
    size_t size = check->size();
    
    LlrType prod = boxSum_.prior(*first);
    for (size_t j = 1; j < size-1; ++j) {
      checkMetricTmp[j] = boxSum_.prior(first[j]);
      first[j] = prod;
      prod = boxSum_.sum(prod, checkMetricTmp[j]);
    }
    checkMetricTmp[size-1] = boxSum_.prior(first[size-1]);
    first[size-1] = boxSum_.post(prod);
    prod = checkMetricTmp[size-1];
    for (size_t j = size-2; j > 0; --j) {
      first[j] = boxSum_.post( boxSum_.sum(first[j], prod) );
      prod = boxSum_.sum(prod, checkMetricTmp[j]);
    }
    *first = boxSum_.post(prod);
    
    checkMetric += size;
  }
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::bitUpdate()
{
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  for (size_t i = 0; i < structure().checks().size(); ++i) {
    checkMetricsBuffer_[i] = checkMetrics_[i];
    auto& ref = bitMetrics_[structure().checks().at(i)];
    checkMetrics_[i] = ref;
    ref += checkMetricsBuffer_[i];
  }
  
  std::copy(parity_.begin(), parity_.begin() + bitMetrics_.size(), bitMetrics_.begin());
  
  for (int64_t i = structure().checks().size() - 1; i >= 0; --i) {
    auto& ref = bitMetrics_[structure().checks().at(i)];
    checkMetrics_[i] += ref;
    ref += checkMetricsBuffer_[i];
  }
}

template class fec::BpDecoderImpl<FloatLlrMetrics, BoxSum>;
template class fec::BpDecoderImpl<FloatLlrMetrics, MinBoxSum>;
template class fec::BpDecoderImpl<FloatLlrMetrics, LinearBoxSum>;

