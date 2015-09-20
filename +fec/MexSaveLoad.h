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
 
 Definition of general conversion to and from mxArray
 ******************************************************************************/

#ifndef MEX_SAVE_LOAD
#define MEX_SAVE_LOAD

#include <memory>
#include <cstring>
#include <type_traits>
#include <vector>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <mex.h>

#include "Structure/Serialization.h"
#include "MexConversion.h"

template <class T, class DerivedTypeHolder>
mxArray* save(const MexHandle<T>& u,  DerivedTypeHolder derived)
{
  const T* base_pointer = u.get();
  
  size_t size = archiveSize(base_pointer, derived);
  mxArray* out = mxCreateNumericMatrix(size, 1, MexType<uint8_t>::ID::value, mxREAL);
  save(base_pointer, reinterpret_cast<char*>(mxGetData(out)), size, derived);
  
  return out;
}

template <class T, class DerivedTypeHolder>
MexHandle<T> load(const mxArray* in, DerivedTypeHolder derived)
{
  if (in == nullptr) {
    std::invalid_argument("null");
  }
  if (mxGetData(in) == nullptr) {
    std::invalid_argument("invalid");
  }
  auto ptr = load<T>(reinterpret_cast<const char*>(mxGetData(in)), mxGetNumberOfElements(in), derived);
  return MexHandle<T>(ptr.release());
}

#endif
