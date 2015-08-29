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
 
 Declaration of LogMapDecoder abstract class
 ******************************************************************************/

#ifndef MIN_SUM_BP_H
#define MIN_SUM_BP_H

#include <algorithm>
#include <cmath>

#include "BpDecoderImpl.h"
#include "../LdpcCode.h"

namespace fec {

/**
 *  This class contains implementation of MinSum approximation for boxplus operation.
 */
class MinSumBp {
  friend class BpDecoderImpl<MinSumBp>;
public:
  
private:
  static inline void checkMetric(std::vector<LlrType>::iterator first, std::vector<LlrType>::iterator last, std::vector<LlrType>::iterator buffer) {
    bool sign = false;
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
    }
  }
  
  static inline LlrType f(LlrType x)
  {
    return x;
  }
  static inline LlrType b(LlrType x)
  {
    return x;
  }
  static inline LlrType step(LlrType a, LlrType b)
  {
    if (std::signbit(a) ^ std::signbit(b)) {
      return std::min(fabs(a), fabs(b));
    }
    else {
      return -std::min(fabs(a), fabs(b));
    }
  }
};
  
}

#endif
