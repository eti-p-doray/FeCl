/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
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
 
 Declaration of LlrMetrics class
 ******************************************************************************/

#ifndef FEC_LLR_METRICS_H
#define FEC_LLR_METRICS_H

#include <type_traits>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>

#include "LinearTable.h"

#undef max

//chanco
namespace fec {
  
  using LlrType = double;
  
  class FloatLlrMetrics {
  public:
    using Type = double;
    static inline Type max() {return std::numeric_limits<Type>::infinity();}
  };
  
  /**
   *  This class contains implementation of log sum operation.
   */
  template <typename LlrMetrics>
  class LogSum {
  public:
    using isRecursive = std::false_type;
    
    /**
     * Computes log sum operation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type max(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return std::max(a,b);}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x, typename LlrMetrics::Type max) {
      if (x == max) {
        return 1.0;
      }
      return std::exp(x - max);
    }
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return a+b;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x, typename LlrMetrics::Type max) {return std::log(x) + max;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
  };
  
  /**
   *  This class contains implementation of the max approximation for log add operation.
   */
  template <typename LlrMetrics>
  class MaxLogSum {
  public:
    using isRecursive = std::true_type;
    //typedef std::false_type isRecursive;
    
    /**
     * Computes log add operation with max approximation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return std::max(a,b);}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
  };
  
  /**
   *  This class contains implementation of the max approximation for log add operation.
   */
  template <typename LlrMetrics>
  class LinearLogSum {
  public:
    using isRecursive = std::true_type;
    
    LinearLogSum(typename LlrMetrics::Type step = 4.0, size_t length = 8) : log1pexpm(step, length) {}
    /**
     * Computes log add operation with max approximation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {
      if (a == b) {
        return a;
      }
      return std::max(a,b) + log1pexpm(std::abs(a-b));
    }
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
  
  private:
    Linearlog1pexpm<typename LlrMetrics::Type> log1pexpm;
  };
  
  /**
   *  This class contains implementation of the max approximation for log add operation.
   */
  template <typename LlrMetrics>
  class BoxSum {
  public:
    using isRecursive = std::true_type;
    //typedef std::false_type isRecursive;
    
    /**
     * Computes log add operation with max approximation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return a*b;}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return tanh(-x/2.0);}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return -std::log((1.0+x)/(1.0-x));}//{return -2.0*atanh(x);}
  };
  
  template <typename LlrMetrics>
  class LinearBoxSum {
  public:
    using isRecursive = std::true_type;
    
    LinearBoxSum(typename LlrMetrics::Type step = 4.0, size_t length = 8) : log1pexpm(step, length) {}
    /**
     * Computes log add operation with max approximation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {
      if (std::signbit(a) ^ std::signbit(b)) {
        return std::min(std::abs(a),std::abs(b)) + log1pexpm(std::abs(a+b)) - log1pexpm(std::abs(a-b));
      }
      else {
        return -std::min(std::abs(a),std::abs(b)) + log1pexpm(std::abs(a+b)) - log1pexpm(std::abs(a-b));
      }
    }
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
    
  private:
    Linearlog1pexpm<typename LlrMetrics::Type> log1pexpm;
  };

  
  /**
   *  This class contains implementation of the max approximation for log add operation.
   */
  template <typename LlrMetrics>
  class MinBoxSum {
  public:
    using isRecursive = std::true_type;
    //typedef std::false_type isRecursive;
    
    /**
     * Computes log add operation with max approximation.
     *  \param  a First operand
     *  \param  b Second operand
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {
      if (std::signbit(a) ^ std::signbit(b)) {
        return std::min(fabs(a), fabs(b));
      }
      else {
        return -std::min(fabs(a), fabs(b));
      }
    }
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
  };
  
  /**
   *  Computes the probability (L-value) of a sequence of input L-values
   *  related to a sequence of bits.
   *  The answer is defined as the correlations between the two inputs.
   *  \param  a Sequence of bits as a BitField
   *  \param  b Random access input iterator associated with the sequence of L-values
   *  \return Correlation between the two inputs
   */
  template <class LlrMetrics>
  inline typename LlrMetrics::Type correlation(const fec::BitField<size_t>& a, typename std::vector<LlrType>::const_iterator b, size_t size) {
    typename LlrMetrics::Type x = 0;
    for (size_t i = 0; i < size; ++i) {
      if (a.test(i)) {
        x += typename LlrMetrics::Type(b[i]);
      }
    }
    return x;
  }
  
  
}

#endif