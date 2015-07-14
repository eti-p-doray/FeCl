/*******************************************************************************
 *  \file MexConversion.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Definition of general conversion to and from mxArray
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
      ptr_ = mxMalloc(size);
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

#endif
