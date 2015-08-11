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

#ifndef LOG_MAP_H
#define LOG_MAP_H

#include <algorithm>
#include <cmath>

#include "MapDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/**
 *  This class contains implementation of log add operation.
 */
class LogMap {
  friend class MapDecoderImpl<LogMap>;
public:
  
private:
  /**
   * Computes log add operation.
   *  \param  a First operand
   *  \param  b Second operand
   */
  static inline LlrType logAdd(LlrType a, LlrType b) {
    LlrType sum = std::max(a,b);
    if (sum == -MAX_LLR) {
      return sum;
    }
    return sum + log(1.0 + exp(-fabs(a-b)));
  }
};
  
}

#endif
