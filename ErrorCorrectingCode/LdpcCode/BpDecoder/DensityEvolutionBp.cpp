/*******************************************************************************
 *  \file BpDecoder.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-16
 *
 *  Declaration of BpDecoder abstract class
 ******************************************************************************/

#include "DensityEvolutionBp.h"

std::unique_ptr<DensityEvolutionBp> DensityEvolutionBp::create(const LdpcCodeStructure& codeStructure)
{
  return std::unique_ptr<DensityEvolutionBp>(new DensityEvolutionBp(codeStructure));
}

void DensityEvolutionBp::predictNBloc(std::vector<LlrPdf>::const_iterator parityIn, std::vector<LlrPdf>::iterator parityOut, std::vector<LlrPdf>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    predictBloc(parityIn, parityOut, messageOut);
    parityIn += codeStructure_.paritySize();
    parityOut += codeStructure_.paritySize();
    messageOut += codeStructure_.messageSize();
  }
}

void DensityEvolutionBp::predictBloc(std::vector<LlrPdf>::const_iterator parityIn, std::vector<LlrPdf>::iterator parityOut, std::vector<LlrPdf>::iterator messageOut)
{
  if (codeStructure().iterationCount() == 0) {
    std::copy(parityIn, parityIn+codeStructure().messageSize(), messageOut);
    std::copy(parityIn, parityIn+codeStructure().paritySize(), parityOut);
    return;
  }
  
  auto check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      *checkMetric = parityIn[*checkBit];
    }
  }
  
  for (size_t i = 0; i < codeStructure().iterationCount() - 1; ++i) {
    checkUpdate();
    bitUpdate(parityIn);
  }
  checkUpdate();
  
  std::copy(parityIn, parityIn+codeStructure().messageSize(), messageOut);
  std::copy(parityIn, parityIn+codeStructure().paritySize(), parityOut);
  check = codeStructure().parityCheck().cbegin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    for (auto checkBit = check->begin(); checkBit < check->end(); ++checkMetric, ++checkBit) {
      if (*checkBit < codeStructure().messageSize()) {
        messageOut[*checkBit] += *checkMetric;
      }
      parityOut[*checkBit] += *checkMetric;
    }
  }
}

DensityEvolutionBp::DensityEvolutionBp(const LdpcCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  checkMetrics_.resize(codeStructure_.parityCheck().size());
  checkMetricsBuffer_.resize(codeStructure_.parityCheck().size());
  bitMetrics_.resize(codeStructure_.paritySize());

}

void DensityEvolutionBp::checkUpdate()
{
  auto check = codeStructure().parityCheck().begin();
  auto checkMetricTmp = checkMetricsBuffer_.begin();
  for (auto checkMetric = checkMetrics_.begin(); checkMetric < checkMetrics_.end();  ++check) {
    TrueBp::checkMetric(checkMetric, checkMetric + check->size(), checkMetricTmp);
    for (auto checkBit = check->begin(); checkBit < check->end(); ++check) {
      checkBit
      checkMetric[k] +
      TrueBp::checkMetric(checkMetric, checkMetric + check->size(), checkMetricTmp);
      checkMetric[k] -
      TrueBp::checkMetric(checkMetric, checkMetric + check->size(), checkMetricTmp);
    }
    checkMetric += check->size();
  }
}

void DensityEvolutionBp::bitUpdate(std::vector<LlrPdf>::const_iterator parity)
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
