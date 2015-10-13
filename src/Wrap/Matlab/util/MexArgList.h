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

#ifndef MEX_ARG_LIST
#define MEX_ARG_LIST

#include <memory>
#include <type_traits>

#include <mex.h>

template <typename T>
class MexArgList {
public:
  MexArgList(int size, T list[]) : list_(list) {size_ = size;}
  
  size_t size() const {return size_;}
  const mxArray* operator[](size_t i) const {
    if (i > size()) {
      throw std::invalid_argument("Wrong arg count");
    }
    return list_[i];
  }
  mxArray*& operator[](size_t i) {
    if (i > size()) {
      throw std::invalid_argument("Wrong arg count");
    }
    return list_[i];
  }
  
private:
  size_t size_;
  T* list_;
};

#endif
