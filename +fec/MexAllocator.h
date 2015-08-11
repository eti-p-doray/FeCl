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
 
 Definition of the mex allocator.
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
  inline T* allocate(size_t size) throw(std::bad_alloc())
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
  
  inline bool operator==(MexAllocator const& b) { return false; }
  inline bool operator!=(MexAllocator const& b) { return !operator==(b); }
  
private:
  void* ptr_ = nullptr;
  size_t size_ = 0;
};    //    end of class Allocator

template <typename T, class Enable = void>
struct Allocator
{
  typedef std::allocator<T> type;
};

template <typename T>
struct Allocator<T, typename std::enable_if<std::is_convertible<typename std::vector<T,MexAllocator<T>>::iterator, typename std::vector<T>::iterator>::value>::type>
{
  typedef MexAllocator<T> type;
};


#endif
