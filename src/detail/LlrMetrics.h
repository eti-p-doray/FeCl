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

#ifndef FEC_DETAIL_LLR_METRICS_H
#define FEC_DETAIL_LLR_METRICS_H

#include <type_traits>
#include <cmath>
#include <limits>
#include <algorithm>

#include "../BitField.h"
#include "LinearTable.h"

#undef max

namespace fec {
  
  namespace detail {
    
    template <DecoderAlgorithm algorithm, class T>
    class LogSum {};
    
    /**
     *  This class contains implementation of log sum operation.
     */
    template <class T>
    class LogSum<Exact, T> {
    public:
      using isRecursive = std::false_type;
      
      static inline T prior(T x, T max) {
        if (x == max) return 1.0;
        return std::exp(x - max);
      }
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator () (T a, T b) {return a+b;}
      static inline T post(T x, T max) {return std::log(x) + max;}
    };
    
    /**
     *  This class contains implementation of the max approximation for log sum operation.
     */
    template <class T>
    class LogSum<Approximate, T> {
    public:
      using isRecursive = std::true_type;
      
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator() (T a, T b) {return std::max(a,b);}
    };
    
    /**
     *  This class contains implementation of the piece-wise linear approximation for log sum operation.
     */
    template <class T>
    class LogSum<Linear, T> {
    public:
      using isRecursive = std::true_type;
      
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator() (T a, T b) {
        if (a == b) return a;
        return std::max(a,b) + log1pexpm(std::abs(a-b));
      }
      
    private:
      static const Linearlog1pexpm<T> log1pexpm;
    };
    
    template <class T> const Linearlog1pexpm<T> LogSum<Linear, T>::log1pexpm = {};
    
    template <DecoderAlgorithm algorithm, class T>
    class BoxSum {};
    
    /**
     *  This class contains implementation of the box sum operation.
     */
    template <class T>
    class BoxSum<Exact, T> {
    public:
      using isRecursive = std::true_type;
      
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator () (T a, T b) {return a*b;}
      static inline T prior(T x) {return tanh(-x/2.0);}
      static inline T post(T x) {return -std::log((1.0+x)/(1.0-x));}//{return -2.0*atanh(x);}
    };
    
    template <class T>
    class BoxSum<Linear, T> {
    public:
      using isRecursive = std::true_type;
      
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator() (T a, T b) {
        if (std::signbit(a) ^ std::signbit(b)) {
          return std::min(std::abs(a),std::abs(b)) - log1pexpm(std::abs(a+b)) + log1pexpm(std::abs(a-b));
        } else {
          return -std::min(std::abs(a),std::abs(b)) - log1pexpm(std::abs(a+b)) + log1pexpm(std::abs(a-b));
        }
      }
      static inline T prior(T x) {return x;}
      static inline T post(T x) {return x;}
      
    private:
      static const Linearlog1pexpm<T> log1pexpm;
    };
    
    template <class T> const Linearlog1pexpm<T> BoxSum<Linear, T>::log1pexpm = {};
    
    
    /**
     *  This class contains implementation of the min approximation for box sum operation.
     */
    template <class T>
    class BoxSum<Approximate, T> {
    public:
      using isRecursive = std::true_type;
      
      /**
       * Computes log sum operation.
       *  \param  a Left-hand operand.
       *  \param  b Right-hand operand.
       */
      inline T operator() (T a, T b) {
        if (std::signbit(a) ^ std::signbit(b)) {
          return std::min(fabs(a), fabs(b));
        } else {
          return -std::min(fabs(a), fabs(b));
        }
      }
      static inline T prior(T x) {return x;}
      static inline T post(T x) {return x;}
    };
    
    template <class InputIterator>
    inline typename InputIterator::value_type sqDistance(InputIterator a, InputIterator b, size_t size) {
      typename InputIterator::value_type x = 0;
      for (size_t i = 0; i < size; ++i) {
        x += std::pow(a[i] - b[i], 2);
      }
      return x;
    }
    
    template <class InputIterator>
    typename InputIterator::value_type mergeMetrics(InputIterator metric, size_t inputWidth, size_t outputWidth, size_t i) {
      typename InputIterator::value_type x = 0;
      BitField<size_t> a = i;
      for (size_t j = 0; j < outputWidth; j += inputWidth, metric += inputWidth) {
        if (a.test(j, inputWidth)) {
          x += metric[a.test(j, inputWidth)-1];
        }
      }
      return x;
    }
    
    template <DecoderAlgorithm algorithm, class InputIterator, class T = typename InputIterator::value_type, class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
    T splitMetrics(LogSum<algorithm,T> op, InputIterator metric, size_t inputWidth, size_t outputWidth, size_t i, size_t j) {
      auto x = -std::numeric_limits<T>::infinity();
      for (BitField<size_t> k = 1; k < (1<<inputWidth); ++k) {
        if (k.test(i, outputWidth) == j) {
          x = op(x, metric[k]);
        }
      }
      return x - std::log(inputWidth/outputWidth);
    }
    
    template <DecoderAlgorithm algorithm, class InputIterator, class T = typename InputIterator::value_type, class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
    T splitMetrics(LogSum<algorithm,T> op, InputIterator metric, size_t inputWidth, size_t outputWidth, size_t i, size_t j) {
      auto max = -std::numeric_limits<T>::infinity();
      for (BitField<size_t> k = 1; k < (2<<inputWidth); ++k) {
        if (k.test(j, outputWidth) == i) {
          max = std::max(max, metric[k]);
        }
      }
      T x = {};
      for (BitField<size_t> k = 1; k < (2<<inputWidth); ++k) {
        if (k.test(j, outputWidth) == i) {
          x = op(x, op.prior(metric[k], max));
        }
      }
      return op.post(x, max) - std::log(inputWidth/outputWidth);
    }
    
  }
  
}

#endif