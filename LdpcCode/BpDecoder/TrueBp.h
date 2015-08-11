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
    LlrType sum = *first;
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
    *first = sum;
  }
     
  static inline LlrType boxPlus(LlrType a, LlrType b) {
    if (std::signbit(a) ^ std::signbit(b)) {
      return std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
    else {
      return -std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
  }
};
  
}

#endif
