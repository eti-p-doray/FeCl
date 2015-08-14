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
 
 Declaration of TrueBp class
 ******************************************************************************/

#ifndef TRUE_BP_H
#define TRUE_BP_H

#include <algorithm>
#include <math.h>

#include "BpDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/**
 *  This class contains implementation of boxplus operation.
 */
class TrueBp {
  friend class BpDecoderImpl<TrueBp>;
  friend class DensityEvolutionBp;
public:
  
private:
  static inline void checkMetric(std::vector<LlrType>::iterator first, std::vector<LlrType>::iterator last, std::vector<LlrType>::iterator buffer)
  {
    /*LlrType sum = *first;
    auto tmp = buffer + 1;
    for (auto metric = first+1; metric < last-1; ++metric, ++tmp) {
      *tmp = *metric;
      *metric = sum;
      sum = boxPlus(sum, *tmp);
    }
    std::swap(sum, *(last-1));
    --tmp;
    for (auto metric = last-2; metric > first; --metric, --tmp) {
      *metric = boxPlus(sum, *metric);
      sum = boxPlus(sum, *tmp);
    }
    *first = sum;*/
    
    auto tmp = buffer;
    for (auto metric = first; metric < last; ++metric, ++tmp) {
      *tmp = stanh(-*metric/2.0);
    }
    
    LlrType prod = *buffer;
    tmp = buffer + 1;
    for (auto metric = first+1; metric < last-1; ++metric, ++tmp) {
      *metric = prod;
      prod *= *tmp;
    }
    *(last-1) = -2.0*satanh(prod);
    prod = *tmp;
    --tmp;
    for (auto metric = last-2; metric > first; --metric, --tmp) {
      *metric *= prod;
      *metric = -2.0*satanh(*metric);
      prod *= *tmp;
    }
    *first = -2.0*satanh(prod);
  }
  
  /*bool sign = false;
  std::vector<LlrType>::iterator min[2];
  min[0] = first;
  for (auto metricIt = first; metricIt < last; ++metricIt) {
    sign ^= !std::signbit(*metricIt);
    if (fabs(*metricIt) < fabs(*min[0])) {
      min[0] = metricIt;
    }
  }
  min[1] = min[0] == first ? first+1 : first;
  for (auto metricIt = first; metricIt < last; ++metricIt) {
    if (fabs(*metricIt) < fabs(*min[1]) && metricIt != min[0]) {
      min[1] = metricIt;
    }
  }
  
  LlrType minValue[2] = {*min[0], *min[1]};
  for (auto metricIt = first; metricIt < last; ++metricIt) {
    LlrType currentMin = fabs(minValue[0]);
    if (metricIt == min[0]) {
      currentMin = fabs(minValue[1]);
    }
    if (sign ^ !std::signbit(*metricIt)) {
      *metricIt = currentMin;
    }
    else {
      *metricIt = -currentMin;
    }
  }*/
  
  static inline LlrType stanh(LlrType x) {
    if (x >= 19.07) {
      return 1.0;
    }
    else if (x <= -19.07) {
      return -1.0;
    }
    else {
      return tanh(x);
    }
  }
  
  static inline LlrType satanh(LlrType x) {
    if (x >= 1.0) {
      return 19.07;
    }
    else if (x <= -1) {
      return -19.07;
    }
    else {
      return atanh(x);
    }
  }
  
  static inline LlrType boxPlus(LlrType a, LlrType b) {
    if (std::signbit(a) ^ std::signbit(b)) {
      return std::min(std::abs(a),std::abs(b)) - log1pexp(-std::abs(a+b)) + log1pexp(-std::abs(a-b));
      //return std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
    else {
      return -std::min(std::abs(a),std::abs(b)) - log1pexp(-std::abs(a+b)) + log1pexp(-std::abs(a-b));
      //return -std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
  }
  static inline LlrType log1pexp(LlrType x) {
    if (x < -37) {
      return exp(x);
    }
    else {
      return log1p(exp(x));
    }
  }
};
  
}

#endif
