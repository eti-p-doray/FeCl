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
#include "../LdpcCode.h"

namespace fec {

/**
 *  This class contains implementation of boxplus operation.
 */
class TrueBp {
public:
  static inline void checkMetric(std::vector<LlrType>::iterator first, std::vector<LlrType>::iterator last, std::vector<LlrType>::iterator buffer)
  {
    auto tmp = buffer;
    for (auto metric = first; metric < last; ++metric, ++tmp) {
      *tmp = tanh(-*metric/2.0);
    }
    
    LlrType prod = *buffer;
    tmp = buffer + 1;
    for (auto metric = first+1; metric < last-1; ++metric, ++tmp) {
      *metric = prod;
      prod *= *tmp;
    }
    *(last-1) = -2.0*atanh(prod);
    prod = *tmp;
    --tmp;
    for (auto metric = last-2; metric > first; --metric, --tmp) {
      *metric *= prod;
      *metric = -2.0*atanh(*metric);
      prod *= *tmp;
    }
    *first = -2.0*atanh(prod);
  }
  
  static inline LlrType f(LlrType x)
  {
    return tanh(-x/2.0);
  }
  static inline LlrType b(LlrType x)
  {
    return -2.0*atanh(x);
  }
  static inline LlrType step(LlrType a, LlrType b)
  {
    return a*b;
  }
};
  
}

#endif
