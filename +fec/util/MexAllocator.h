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

#ifndef MEX_ALLOCATOR
#define MEX_ALLOCATOR

#include <memory>
#include <type_traits>

#include <mex.h>

template<typename T>
class MexAllocator {
public :
  typedef T value_type;
  typedef std::true_type propagate_on_container_copy_assignment;
  typedef std::true_type propagate_on_container_move_assignment;
  typedef std::true_type propagate_on_container_swap;
  
  inline MexAllocator() = default;
  inline MexAllocator(const mxArray* array) {size_ = mxGetNumberOfElements(array); ptr_ = mxGetPr(array);}
  inline MexAllocator(const MexAllocator&) = default;
  inline ~MexAllocator() {}
  
  inline MexAllocator select_on_container_copy_construction() const {return MexAllocator();}
  
  //    memory allocation
  inline T* allocate(size_t size)
  {
    if (ptr_ == nullptr) {
      ptr_ = mxMalloc(size * sizeof(T) );
      if (ptr_ == nullptr) {
        throw std::bad_alloc();
      }
      size_ = size;
    }
    else if (size > size_) {
      throw std::bad_alloc();
    }

    return reinterpret_cast<T*>(ptr_);
  }
  inline void deallocate(T* p, size_t size) {}
  
  inline void construct(T* x) {}
  template <typename... Args> inline void construct(T* x, Args... args) {*x = T(args...);}
  
  inline T* ptr() {return reinterpret_cast<T*>(ptr_);}
  inline size_t size() const {return size_;}
  
  inline bool operator==(const MexAllocator& b) const { return false; }
  inline bool operator!=(const MexAllocator& b) const { return !operator==(b); }
  
private:
  void* ptr_ = nullptr;
  size_t size_ = 0;
};    //    end of class Allocator

template <typename T, class Enable = void>
struct Allocator
{
  using type = std::allocator<T>;
};

template <typename T>
struct Allocator<T, typename std::enable_if<std::is_convertible<typename std::vector<T,MexAllocator<T>>::iterator, typename std::vector<T>::iterator>::value>::type>
{
  using type = MexAllocator<T>;
};


#endif
