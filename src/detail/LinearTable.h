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

#ifndef FEC_DETAIL_LINEAR_TABLE_H
#define FEC_DETAIL_LINEAR_TABLE_H

#include <cmath>
#include <vector>

namespace fec {
  
  namespace detail {
  
  template <typename T, size_t length>
  class LinearTable {
  public:
    template <class F>
    LinearTable(F f) {
      //y.resize(length);
      for (size_t i = 0; i < y.size(); ++i) {
        y[i] = f(i);
      }
    }
    
    inline T operator () (T x) const {
      size_t i = x;
      return (y[i+1] - y[i]) * (x-i) + y[i];
    }
    inline size_t size() const {
      return y.size();
    }
    
  private:
    //std::vector<T> y;
    std::array<T, length> y;
  };
  
  template <typename T>
  struct Linearlog1pexpm {
    struct Impl {
      Impl(T step) {
        step_ = step;
      }
      T operator()(T x) const {
        return std::log(1.0+std::exp(-(double(x)/step_)));
      }
      T step_;
    };
    
    inline T operator()(T x) const {
      x *= granularity_;
      if(x >= table_.size()-1) {
        return 0;
      }
      return table_(x);
    }
    
    constexpr static T granularity_ = 2.0;
    constexpr static size_t length_ = 8;
    const static LinearTable<T, length_> table_;
  };
  
  template <typename T> const LinearTable<T, Linearlog1pexpm<T>::length_> Linearlog1pexpm<T>::table_ = LinearTable<T, length_>(Impl(granularity_));
  
  }
  
}

#endif