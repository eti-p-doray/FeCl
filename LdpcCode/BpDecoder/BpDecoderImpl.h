/*******************************************************************************
 *  \file BpDecodeImpl.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-16
 *
 *  Declaration of BpDecoderImpl class
 ******************************************************************************/

#ifndef BP_DECODER_IMPL_H
#define BP_DECODER_IMPL_H

#include <algorithm>
#include <math.h>

#include "BpDecoder.h"

namespace fec {

/*******************************************************************************
 *  This class represents a belief propagation decoder implementation.
 *  The reason for this class is to offer an common interface of map decoders 
 *  while allowing the compiler to inline implementation specific functions 
 *  by using templates instead of polymorphism.
 ******************************************************************************/
template <typename A>
class BpDecoderImpl : public BpDecoder {
public:
  BpDecoderImpl(const LdpcCodeStructure& codeStructure);
  ~BpDecoderImpl() = default;
  
protected:
  virtual void checkUpdate();
  virtual void bitUpdate(boost::container::vector<LlrType>::const_iterator parity);
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
void BpDecoderImpl<A>::bitUpdate(boost::container::vector<LlrType>::const_iterator parity)
{
  std::fill(bitMetrics_.begin(), bitMetrics_.end(), 0);
  auto check = codeStructure().parityCheck().begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkMetricTmp, ++checkBit) {
      *checkMetricTmp = *checkMetric;
      *checkMetric = bitMetrics_[*checkBit];
      bitMetrics_[*checkBit] += *checkMetricTmp;
    }
  }
  
  for (auto bitMetric = bitMetrics_.begin(); bitMetric < bitMetrics_.end(); ++bitMetric, ++parity) {
    *bitMetric = *parity;
  }
  
  check = codeStructure().parityCheck().end() - 1;
  checkMetricTmp = checkMetricsBuffer_.end() - 1;
  for (auto checkMetric = checkMetrics_.end() - 1; checkMetric >= checkMetrics_.begin();  --check) {
    for (auto checkBit = check->end()-1; checkBit >= check->begin(); --checkMetric, --checkMetricTmp, --checkBit) {
      *checkMetric += bitMetrics_[*checkBit];
      bitMetrics_[*checkBit] += *checkMetricTmp;
    }
  }
}
  
}

#endif
