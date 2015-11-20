/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
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

namespace fec {
  
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
    
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type max(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return std::max(a,b);}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x, typename LlrMetrics::Type max) {
      if (x == max) {
        return 1.0;
      }
      return std::exp(x - max);
    }
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return a+b;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x, typename LlrMetrics::Type max) {return std::log(x) + max;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type scale(typename LlrMetrics::Type x) {return x;}
  };
  
  /**
   *  This class contains implementation of the max approximation for log sum operation.
   */
  template <typename LlrMetrics>
  class MaxLogSum {
  public:
    using isRecursive = std::true_type;
    
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return std::max(a,b);}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
    inline typename LlrMetrics::Type scale(typename LlrMetrics::Type x) const {return x*scalingFactor_;}
    
  private:
    typename LlrMetrics::Type scalingFactor_ = 1.0;
  };
  
  /**
   *  This class contains implementation of the piece-wise linear approximation for log sum operation.
   */
  template <typename LlrMetrics>
  class LinearLogSum {
  public:
    using isRecursive = std::true_type;
    
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {
      if (a == b) {
        return a;
      }
      return std::max(a,b) + log1pexpm(std::abs(a-b));
    }
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type scale(typename LlrMetrics::Type x) {return x;}
  
  private:
    static const Linearlog1pexpm<typename LlrMetrics::Type> log1pexpm;
  };
  
  template <typename LlrMetrics> const Linearlog1pexpm<typename LlrMetrics::Type> LinearLogSum<LlrMetrics>::log1pexpm = {};
  
  /**
   *  This class contains implementation of the box sum operation.
   */
  template <typename LlrMetrics>
  class BoxSum {
  public:
    using isRecursive = std::true_type;
    
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {return a*b;}
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return tanh(-x/2.0);}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return -std::log((1.0+x)/(1.0-x));}//{return -2.0*atanh(x);}
    static inline typename LlrMetrics::Type scale(typename LlrMetrics::Type x) {return x;}
  };
  
  template <typename LlrMetrics>
  class LinearBoxSum {
  public:
    using isRecursive = std::true_type;
    
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
     */
    static inline typename LlrMetrics::Type sum(typename LlrMetrics::Type a, typename LlrMetrics::Type b) {
      if (std::signbit(a) ^ std::signbit(b)) {
        return std::min(std::abs(a),std::abs(b)) - log1pexpm(std::abs(a+b)) + log1pexpm(std::abs(a-b));
      }
      else {
        return -std::min(std::abs(a),std::abs(b)) - log1pexpm(std::abs(a+b)) + log1pexpm(std::abs(a-b));
      }
    }
    static inline typename LlrMetrics::Type prior(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type post(typename LlrMetrics::Type x) {return x;}
    static inline typename LlrMetrics::Type scale(typename LlrMetrics::Type x) {return x;}
    
  private:
    static const Linearlog1pexpm<typename LlrMetrics::Type> log1pexpm;
  };
  
  template <typename LlrMetrics> const Linearlog1pexpm<typename LlrMetrics::Type> LinearBoxSum<LlrMetrics>::log1pexpm = {};

  
  /**
   *  This class contains implementation of the min approximation for box sum operation.
   */
  template <typename LlrMetrics>
  class MinBoxSum {
  public:
    using isRecursive = std::true_type;
    
    /**
     * Computes log sum operation.
     *  \param  a Left-hand operand.
     *  \param  b Right-hand operand.
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
    
  private:
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
  inline typename LlrMetrics::Type correlation(const fec::BitField<size_t>& a, typename std::vector<double>::const_iterator b, size_t size) {
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