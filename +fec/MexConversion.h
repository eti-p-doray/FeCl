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

#ifndef MEX_CONVERSION
#define MEX_CONVERSION

#include <memory>
#include <cstring>
#include <type_traits>
#include <vector>

#include <boost/serialization/type_info_implementation.hpp>

#include <mex.h>

#include "MexAllocator.h"
#include "MexHandle.h"

template <typename T> struct MexType {using ID = std::integral_constant<mxClassID, mxUNKNOWN_CLASS>; using isScalar = std::false_type;};

template <> struct MexType<bool> {using ID = std::integral_constant<mxClassID, mxLOGICAL_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<int8_t> {using ID = std::integral_constant<mxClassID, mxINT8_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<uint8_t> {using ID = std::integral_constant<mxClassID, mxUINT8_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<int16_t> {using ID = std::integral_constant<mxClassID, mxINT16_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<uint16_t> {using ID = std::integral_constant<mxClassID, mxUINT16_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<int32_t> {using ID = std::integral_constant<mxClassID, mxINT32_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<uint32_t> {using ID = std::integral_constant<mxClassID, mxUINT32_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<int64_t> {using ID = std::integral_constant<mxClassID, mxINT64_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<uint64_t> {using ID = std::integral_constant<mxClassID, mxUINT64_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<size_t> {using ID = std::integral_constant<mxClassID, mxUINT64_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<float> {using ID = std::integral_constant<mxClassID, mxSINGLE_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<double> {using ID = std::integral_constant<mxClassID, mxDOUBLE_CLASS>; using isScalar = std::true_type;};

template <class T, class isEnum = void, class isScalar = void>
class mxArrayTo {};

template <class T>
class mxArrayTo<T, void, typename std::enable_if<MexType<T>::isScalar::value>::type>
{
public:
  static T f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (mxIsComplex(in) || mxGetNumberOfElements(in) != 1 || mxGetData(in) == nullptr) {
      throw std::invalid_argument("invalid");
    }
    switch (mxGetClassID(in))
    {
      case mxLOGICAL_CLASS:
        return *reinterpret_cast<mxLogical*>(mxGetData(in));
      case mxINT8_CLASS:
        return *reinterpret_cast<int8_t*>(mxGetData(in));
      case mxUINT8_CLASS:
        return *reinterpret_cast<uint8_t*>(mxGetData(in));
      case mxINT16_CLASS:
        return *reinterpret_cast<int16_t*>(mxGetData(in));
      case mxUINT16_CLASS:
        return *reinterpret_cast<uint16_t*>(mxGetData(in));
      case mxINT32_CLASS:
        return *reinterpret_cast<int32_t*>(mxGetData(in));
      case mxUINT32_CLASS:
        return *reinterpret_cast<uint32_t*>(mxGetData(in));
      case mxINT64_CLASS:
        return *reinterpret_cast<int64_t*>(mxGetData(in));
      case mxUINT64_CLASS:
        return *reinterpret_cast<uint64_t*>(mxGetData(in));
      case mxSINGLE_CLASS:
        return *reinterpret_cast<float*>(mxGetData(in));
      case mxDOUBLE_CLASS:
        return *reinterpret_cast<double*>(mxGetData(in));
      default:
        throw std::invalid_argument("unknown");
    }
  }
};

template <typename T>
class mxArrayTo<T, typename std::enable_if<std::is_enum<T>::value>::type, void> {
public:
  static T f(const mxArray* in) {
    return static_cast<T>(mxArrayTo<size_t>::f(in));
  }
};

template <class T, class A>
class mxArrayTo<std::vector<T,A>, void, typename std::enable_if<MexType<T>::isScalar::value>::type> {
public:
  static std::vector<T,A> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (mxIsComplex(in) || (mxGetData(in) == nullptr && mxGetNumberOfElements(in) != 0)) {
      throw std::invalid_argument("invalid");
    }
    if (mxGetData(in) == nullptr && mxGetNumberOfElements(in) == 0) {
      return std::vector<T,A>();
    }
    std::vector<T,A> vec(mxGetNumberOfElements(in));
    if (mxIsCell(in)) {
      for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = mxArrayTo<T>::f(mxGetCell(in, i));
      }
    }
    else {
      copy(in, vec.begin());
    }
    return vec;
  }
  
  static void copy(const mxArray* in, typename std::vector<T,A>::iterator begin) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr) {
      throw std::invalid_argument("invalid");
    }
    size_t size = mxGetNumberOfElements(in);
    switch (mxGetClassID(in))
    {
      case mxLOGICAL_CLASS:
        std::copy(reinterpret_cast<mxLogical*>(mxGetData(in)), reinterpret_cast<mxLogical*>(mxGetData(in))+size, begin);
        break;
      case mxINT8_CLASS:
        std::copy(reinterpret_cast<int8_t*>(mxGetData(in)), reinterpret_cast<int8_t*>(mxGetData(in))+size, begin);
        break;
      case mxUINT8_CLASS:
        std::copy(reinterpret_cast<uint8_t*>(mxGetData(in)), reinterpret_cast<uint8_t*>(mxGetData(in))+size, begin);
        break;
      case mxINT16_CLASS:
        std::copy(reinterpret_cast<int16_t*>(mxGetData(in)), reinterpret_cast<int16_t*>(mxGetData(in))+size, begin);
        break;
      case mxUINT16_CLASS:
        std::copy(reinterpret_cast<uint16_t*>(mxGetData(in)), reinterpret_cast<uint16_t*>(mxGetData(in))+size, begin);
        break;
      case mxINT32_CLASS:
        std::copy(reinterpret_cast<int32_t*>(mxGetData(in)), reinterpret_cast<int32_t*>(mxGetData(in))+size, begin);
        break;
      case mxUINT32_CLASS:
        std::copy(reinterpret_cast<uint32_t*>(mxGetData(in)), reinterpret_cast<uint32_t*>(mxGetData(in))+size, begin);
        break;
      case mxINT64_CLASS:
        std::copy(reinterpret_cast<int64_t*>(mxGetData(in)), reinterpret_cast<int64_t*>(mxGetData(in))+size, begin);
        break;
      case mxUINT64_CLASS:
        std::copy(reinterpret_cast<uint64_t*>(mxGetData(in)), reinterpret_cast<uint64_t*>(mxGetData(in))+size, begin);
        break;
      case mxSINGLE_CLASS:
        std::copy(reinterpret_cast<float*>(mxGetData(in)), reinterpret_cast<float*>(mxGetData(in))+size, begin);
        break;
      case mxDOUBLE_CLASS:
        std::copy(reinterpret_cast<double*>(mxGetData(in)), reinterpret_cast<double*>(mxGetData(in))+size, begin);
        break;
      default:
        throw std::invalid_argument("unknown");
        break;
    }
  }
};

template <class T>
class mxArrayTo<std::vector<T, MexAllocator<T>>, void, typename std::enable_if<MexType<T>::isScalar::value>::type> {
public:
  static std::vector<T, MexAllocator<T>> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (mxIsComplex(in) || (mxGetData(in) == nullptr && mxGetNumberOfElements(in) != 0)) {
      throw std::invalid_argument("invalid");
    }
    if (mxGetData(in) == nullptr && mxGetNumberOfElements(in) == 0) {
      return std::vector<T,MexAllocator<T>>();
    }
    if (MexType<T>::ID::value != mxGetClassID(in)) {
      std::vector<T, MexAllocator<T>> vec;
      vec.resize(mxGetNumberOfElements(in));
      mxArrayTo<std::vector<T>>::copy(in, vec.begin());
      return vec;
    }
    else {
      std::vector<T, MexAllocator<T>> vec((MexAllocator<T>(in)));
      vec.resize(mxGetNumberOfElements(in));
      return vec;
    }
  }
};

template <class T, class A>
class mxArrayTo<std::vector<T,A>, void, typename std::enable_if<!MexType<T>::isScalar::value>::type> {
public:
  static std::vector<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (!mxIsCell(in)) {
      throw std::invalid_argument("invalid");
    }
    std::vector<T> out(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = mxArrayTo<T>::f(mxGetCell(in, i));
    }
    return out;
  }
};

template <class T>
class mxArrayTo<MexHandle<T>> {
public:
  template <class DerivedTypeHolder>
  static MexHandle<T> f(const mxArray* in, DerivedTypeHolder derived) {
    derived.register_type();
    return mxArrayTo<MexHandle<T>>::f(in);
  }
  static MexHandle<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("null");
    }
    if (mxGetProperty(in, 0, "mexHandle_") == nullptr) {
      throw std::invalid_argument("invalid");
    }
    if (mxGetData(mxGetProperty(in, 0, "mexHandle_")) == nullptr) {
      throw std::invalid_argument("invalid");
    }
    T* ptr = reinterpret_cast<T*>(*((uint64_t *)mxGetData(mxGetProperty(in, 0, "mexHandle_"))));
    ptr = dynamic_cast<T*>(ptr);
    if (ptr == nullptr) {
      throw std::invalid_argument("null");
    }
    if (boost::serialization::type_info_implementation<T>::type::get_const_instance().get_derived_extended_type_info(*ptr) == nullptr) {
      throw std::invalid_argument("invalid");
    }
    return MexHandle<T>(ptr);
  }
};


template <class T, typename std::enable_if<MexType<T>::isScalar::value>::type* = nullptr>
mxArray* toMxArray(const T& in) {
  mxArray* out = mxCreateNumericMatrix(1,1, MexType<T>::ID::value, mxREAL);
  *static_cast<T*>(mxGetData(out)) = in;
  return out;
}

template <class T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
mxArray* toMxArray(const T& in) {
  return toMxArray(static_cast<uint32_t>(in));
}

template <class T, template <class> class A, typename std::enable_if<MexType<T>::isScalar::value>::type* = nullptr>
mxArray* toMxArray(const std::vector<T, A<T>>& vec) {
  mxArray* out = mxCreateNumericMatrix(vec.size(), 1, MexType<T>::ID::value, mxREAL);
  std::copy(vec.begin(), vec.end(), static_cast<T*>(mxGetData(out)));
  return out;
}

template <class T, template <class> class A, typename std::enable_if<!MexType<T>::isScalar::value>::type* = nullptr>
mxArray* toMxArray(const std::vector<T, A<T>>& vec) {
  mxArray* out = mxCreateCellMatrix(vec.size(), 1);
  for (size_t i = 0; i < vec.size(); ++i) {
    mxSetCell(out, i, toMxArray(vec[i]));
  }
  return out;
}

template <class T>
mxArray* toMxArray(const std::vector<T, MexAllocator<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(0, 0, MexType<T>::ID::value, mxREAL);
  mxSetData(out, all.ptr());
  mxSetM(out, all.size());
  mxSetN(out, 1);
  return out;
}

template <class T>
mxArray* toMxArray(MexHandle<T>&& u) {
  mxArray* out = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  *((uint64_t *)mxGetData(out)) = reinterpret_cast<uint64_t>(u.get());
  return out;
}

#endif
