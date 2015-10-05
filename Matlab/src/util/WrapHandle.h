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

#ifndef WRAP_HANDLE
#define WRAP_HANDLE

#include <memory>
#include <type_traits>

#include <mex.h>

template<typename T>
class WrapHandle {
public :
  WrapHandle() = default;
  WrapHandle(T* ptr) {ptr_ = ptr;}
  WrapHandle(WrapHandle&& other) {ptr_ = other.ptr_; other.ptr_ = nullptr;}
  WrapHandle(const WrapHandle&) = delete;
  WrapHandle& operator=(WrapHandle&& other) {ptr_ = other.ptr_; other.ptr_ = nullptr; return *this;}
  ~WrapHandle() = default;
  
  T* get() {return ptr_;}
  const T* get() const {return ptr_;}
  void reset() {delete ptr_; ptr_ = nullptr;}
  
  T& operator*() {return *ptr_;}
  const T& operator*() const {return *ptr_;}
  
  T* operator->() {return ptr_;}
  const T* operator->() const {return ptr_;}
  
private:
  T* ptr_ = nullptr;
};    //    end of class Allocator

#endif
