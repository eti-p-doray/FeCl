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
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
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
  
  bool success = false;
  for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
    checkUpdate();
    bitUpdate();
    
    for (size_t j = 0; j < structure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      success = true;
      break;
    }
  }
  checkUpdate();
  
  std::copy(parity_.begin(), parity_.end(), bitMetrics_.begin());
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

  bool success = false;
  for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
    checkUpdate();
    bitUpdate();
    
    for (size_t j = 0; j < structure().paritySize(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      success = true;
      break;
    }
  }
  checkUpdate();
  
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

