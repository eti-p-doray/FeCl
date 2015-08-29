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
#include <cmath>

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
  BpDecoderImpl(const LdpcCode::Structure& codeStructure);
  ~BpDecoderImpl() = default;
  
protected:
  virtual void checkUpdate();
  virtual void bitUpdate(std::vector<LlrType>::const_iterator parity);
};

template <typename A>
BpDecoderImpl<A>::BpDecoderImpl(const LdpcCode::Structure& codeStructure) : BpDecoder(codeStructure)
{
  
}

template <typename A>
void BpDecoderImpl<A>::checkUpdate()
{
  auto checkMetric = checkMetrics_.begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto check = codeStructure().checks().begin(); check < codeStructure().checks().end();  ++check) {
    auto first = checkMetric;
    size_t size = check->size();
    
    LlrType prod = A::f(*first);
    for (size_t j = 1; j < size-1; ++j) {
      checkMetricTmp[j] = A::f(first[j]);
      first[j] = prod;
      prod = A::step(prod, checkMetricTmp[j]);
    }
    checkMetricTmp[size-1] = A::f(first[size-1]);
    first[size-1] = A::b(prod);
    prod = checkMetricTmp[size-1];
    for (size_t j = size-2; j > 0; --j) {
      first[j] = A::b( A::step(first[j], prod) );
      prod = A::step(prod, checkMetricTmp[j]);
    }
    *first = A::b(prod);
    
    checkMetric += size;
  }
}

template <typename A>
void BpDecoderImpl<A>::bitUpdate(std::vector<LlrType>::const_iterator parity)
{
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  for (size_t i = 0; i < codeStructure().checks().size(); ++i) {
    checkMetricsBuffer_[i] = checkMetrics_[i];
    LlrType& ref = bitMetrics_[codeStructure().checks().at(i)];
    checkMetrics_[i] = ref;
    ref += checkMetricsBuffer_[i];
  }
  
  std::copy(parity, parity + bitMetrics_.size(), bitMetrics_.begin());
  
  for (int64_t i = codeStructure().checks().size() - 1; i >= 0; --i) {
    LlrType& ref = bitMetrics_[codeStructure().checks().at(i)];
    checkMetrics_[i] += ref;
    ref += checkMetricsBuffer_[i];
  }
}
  
}

#endif
