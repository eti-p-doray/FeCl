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

#ifndef BP_DECODER_IMPL_H
#define BP_DECODER_IMPL_H

#include <algorithm>
#include <math.h>

#include "BpDecoder.h"

namespace fec {

  /**
   *  This class contains the concrete implementation of the belief propagation decoder.
   *  This algorithm is used for decoding in an LdpcCode.
   *  The reason for this class is to offer an common interface of bp decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
template <typename A>
class BpDecoderImpl : public BpDecoder {
public:
  BpDecoderImpl(const LdpcCodeStructure& codeStructure);
  ~BpDecoderImpl() = default;
  
protected:
  virtual void checkUpdate();
  virtual void bitUpdate(std::vector<LlrType>::const_iterator parity);
};

template <typename A>
BpDecoderImpl<A>::BpDecoderImpl(const LdpcCodeStructure& codeStructure) : BpDecoder(codeStructure)
{
  
}

template <typename A>
void BpDecoderImpl<A>::checkUpdate()
{
  auto check = codeStructure().parityCheck().begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    A::checkMetric(checkMetric, checkMetric + check->size(), checkMetricTmp);
    checkMetric += check->size();
  }
}

template <typename A>
void BpDecoderImpl<A>::bitUpdate(std::vector<LlrType>::const_iterator parity)
{
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  auto check = codeStructure().parityCheck().begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkMetricTmp, ++checkBit) {
      *checkMetricTmp = *checkMetric;
      LlrType& metricRef = bitMetrics_[*checkBit];
      //*checkMetric = bitMetrics_[*checkBit];
      //bitMetrics_[*checkBit] += *checkMetricTmp;
      *checkMetric = metricRef;
      metricRef += *checkMetricTmp;
    }
  }
  
  std::copy(parity, parity + bitMetrics_.size(), bitMetrics_.begin());
  
  check = codeStructure().parityCheck().end() - 1;
  checkMetricTmp = checkMetricsBuffer_.end() - 1;
  for (auto checkMetric = checkMetrics_.end() - 1; checkMetric >= checkMetrics_.begin();  --check) {
    for (auto checkBit = check->end()-1; checkBit >= check->begin(); --checkMetric, --checkMetricTmp, --checkBit) {
      LlrType& metricRef = bitMetrics_[*checkBit];
      //*checkMetric += bitMetrics_[*checkBit];
      //bitMetrics_[*checkBit] += *checkMetricTmp;
      *checkMetric += metricRef;
      metricRef += *checkMetricTmp;
    }
  }
}
  
}

#endif
