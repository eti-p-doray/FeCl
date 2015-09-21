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

#ifndef FEC_MEX_HANDLE
#define FEC_MEX_HANDLE

#include <memory>
#include <type_traits>

#include <mex.h>

template<typename T>
class MexHandle {
public :
  MexHandle() = default;
  MexHandle(T* ptr) {ptr_ = ptr;}
  MexHandle(MexHandle&& other) {ptr_ = other.ptr_; other.ptr_ = nullptr;}
  MexHandle(const MexHandle&) = delete;
  MexHandle& operator=(MexHandle&& other) {ptr_ = other.ptr_; other.ptr_ = nullptr; return *this;}
  ~MexHandle() = default;
  
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
