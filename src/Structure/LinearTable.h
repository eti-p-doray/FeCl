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
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_LINEAR_TABLE_H
#define FEC_LINEAR_TABLE_H

#include <cmath>
#include <vector>

//chanco
namespace fec {
  
  template <typename T>
  class LinearTable {
  public:
    template <class F>
    LinearTable(size_t lenght, F f) {
      y.resize(lenght);
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
    std::vector<T> y;
  };
  
  template <typename T>
  struct Linearlog1pexpm {
    Linearlog1pexpm(int step, size_t length) : table_(length, Impl(step)) {
      step_ = step;
    }
    struct Impl {
      Impl(T step) {
        step_ = step;
      }
      T operator()(T x) {
        return std::log(1.0+std::exp(-(double(x)/step_)));
      }
      T step_;
    };
    
    inline T operator()(T x) const {
      x *= step_;
      if(x >= table_.size()-1) {
        return 0;
      }
      return table_(x);
    }
    
    T step_;
    LinearTable<T> table_;
  };
  
}

#endif