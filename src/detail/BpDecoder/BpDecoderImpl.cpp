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

#include "BpDecoderImpl.h"

using namespace fec::detail;

template <class LlrMetrics, template <class> class BoxSumAlg>
BpDecoderImpl<LlrMetrics, BoxSumAlg>::BpDecoderImpl(const Ldpc::Structure& structure) :
BpDecoder(structure)
{
  hardParity_.resize(this->structure().checks().cols());
  parity_.resize(this->structure().checks().cols());
  checkMetrics_.resize(this->structure().checks().size());
  checkMetricsBuffer_.resize(this->structure().checks().size());
  bitMetrics_.resize(this->structure().checks().cols());
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::decodeBlock(std::vector<double>::const_iterator parity, std::vector<fec::BitField<size_t>>::iterator msg)
{
  std::copy(parity, parity+structure().checks().cols(), parity_.begin());

  if (structure().iterations() > 0) {
    for (size_t i = 0; i < structure().checks().size(); ++i) {
      checkMetrics_[i] = parity_[structure().checks().at(i)];
    }
  }
  
  bool success = false;
  for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
    checkUpdate(i);
    bitUpdate();
    
    for (size_t j = 0; j < structure().checks().cols(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      success = true;
      break;
    }
  }
  checkUpdate(structure().iterations()-1);
  
  std::copy(parity_.begin(), parity_.end(), bitMetrics_.begin());
  for (size_t i = 0; i < structure().checks().size(); ++i) {
    bitMetrics_[structure().checks().at(i)] += checkMetrics_[i];
  }
  for (size_t i = 0; i < structure().msgSize(); ++i) {
    msg[i] = bitMetrics_[i] >= 0;
  }
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::soDecodeBlock(detail::Codec::const_iterator<double> input, detail::Codec::iterator<double> output)
{
  std::copy(input.at(Codec::Parity), input.at(Codec::Parity)+structure().checks().cols(), parity_.begin());
  if (input.count(Codec::Syst)) {
    auto syst = input.at(Codec::Syst);
    for (size_t i = 0; i < structure().systSize(); ++i) {
      parity_[i] += syst[i];
    }
  }
  if (input.count(Codec::State)) {
    std::copy(input.at(Codec::State), input.at(Codec::State)+structure().stateSize(), checkMetrics_.begin());
  }
  
  if (structure().iterations() > 0) {
    if (input.count(Codec::State)) {
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
    checkUpdate(i);
    bitUpdate();
    
    for (size_t j = 0; j < structure().checks().cols(); ++j) {
      hardParity_[j] = (bitMetrics_[j] >= 0.0);
    }
    if (structure().check(hardParity_.begin())) {
      success = true;
      break;
    }
  }
  checkUpdate(structure().iterations()-1);
  
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  for (size_t i = 0; i < structure().checks().size(); ++i) {
    bitMetrics_[structure().checks().at(i)] += checkMetrics_[i];
  }
  
  if (output.count(Codec::Syst)) {
    std::copy(bitMetrics_.begin(), bitMetrics_.begin()+structure().systSize(), output.at(Codec::Syst));
  }
  if (output.count(Codec::Parity)) {
    std::copy(bitMetrics_.begin(), bitMetrics_.begin()+structure().checks().cols(), output.at(Codec::Parity));
  }
  if (output.count(Codec::State)) {
    std::copy(checkMetrics_.begin(), checkMetrics_.end(), output.at(Codec::State));
  }
  if (output.count(Codec::Msg)) {
    auto msg = output.at(Codec::Msg);
    for (size_t i = 0; i < structure().msgSize(); ++i) {
      msg[i] = parity_[i] + bitMetrics_[i];
    }
  }
}

template <class LlrMetrics, template <class> class BoxSumAlg>
void BpDecoderImpl<LlrMetrics, BoxSumAlg>::checkUpdate(size_t i)
{
  auto checkMetric = checkMetrics_.begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto check = structure().checks().begin(); check < structure().checks().end();  ++check) {
    auto first = checkMetric;
    size_t size = check->size();
    double sf = structure().scalingFactor(i, size);
    
    double prod = boxSum_.prior(*first);
    for (size_t j = 1; j < size-1; ++j) {
      checkMetricTmp[j] = boxSum_.prior(first[j]);
      first[j] = prod;
      prod = boxSum_.sum(prod, checkMetricTmp[j]);
    }
    checkMetricTmp[size-1] = boxSum_.prior(first[size-1]);
    first[size-1] = sf *  (boxSum_.post(prod));
    prod = checkMetricTmp[size-1];
    for (size_t j = size-2; j > 0; --j) {
      first[j] = sf *  (boxSum_.post( boxSum_.sum(first[j], prod) ));
      prod = boxSum_.sum(prod, checkMetricTmp[j]);
    }
    *first = sf *  (boxSum_.post(prod));
    
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

template class fec::detail::BpDecoderImpl<FloatLlrMetrics, BoxSum>;
template class fec::detail::BpDecoderImpl<FloatLlrMetrics, MinBoxSum>;
template class fec::detail::BpDecoderImpl<FloatLlrMetrics, LinearBoxSum>;

