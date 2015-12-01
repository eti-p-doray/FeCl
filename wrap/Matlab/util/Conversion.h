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

#ifndef WRAP_CONVERSION
#define WRAP_CONVERSION

#include <memory>
#include <cstring>
#include <type_traits>
#include <vector>
#include <string>
#include <unordered_map>

#include <boost/serialization/type_info_implementation.hpp>

#include <mex.h>

#include "MexAllocator.h"
#include "MexHandle.h"

template <typename T1, typename T2>
struct is_equiv_int : std::integral_constant<bool,
(sizeof(T1)==sizeof(T2)) &&
(std::is_integral<T1>::value && std::is_integral<T2>::value) &&
(std::is_signed<T1>::value == std::is_signed<T2>::value)>
{};

template<typename T> struct is_vector : public std::false_type {};
template<typename T, typename A>
struct is_vector<std::vector<T, A>> : public std::true_type {};

template <typename T, class Enable = void> struct MexType {using ID = std::integral_constant<mxClassID, mxUNKNOWN_CLASS>; using isScalar = std::false_type;};

template <> struct MexType<bool> {using ID = std::integral_constant<mxClassID, mxLOGICAL_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,int8_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxINT8_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,uint8_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxUINT8_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,int16_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxINT16_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,uint16_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxUINT16_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,int32_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxINT32_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,uint32_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxUINT32_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,int64_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxINT64_CLASS>; using isScalar = std::true_type;};

template <typename T> struct MexType<T, typename std::enable_if<is_equiv_int<T,uint64_t>::value>::type>
{using ID = std::integral_constant<mxClassID, mxUINT64_CLASS>; using isScalar = std::true_type;};

template <> struct MexType<float> {using ID = std::integral_constant<mxClassID, mxSINGLE_CLASS>; using isScalar = std::true_type;};
template <> struct MexType<double> {using ID = std::integral_constant<mxClassID, mxDOUBLE_CLASS>; using isScalar = std::true_type;};

class ExceptionThrower {
public:
  ExceptionThrower(const std::string& msg) {msg_ = msg;}
  ExceptionThrower& operator() (const std::string& msg) {msg_ += "In " + msg + ": ";}
  inline std::string msg() const {return msg_;}

private:
  std::string msg_;
};

template <class T, class Enable = void>
class mxArrayTo {};

template <class T>
class mxArrayTo<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  T operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    if (mxIsComplex(in)) {
      throw std::invalid_argument(msg() + "Input is complex");
    }
    if (mxGetNumberOfElements(in) != 1 || mxGetData(in) == nullptr) {
      throw std::invalid_argument(msg() + "Input is not a scalar");
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
        throw std::invalid_argument(msg() + "Unknown input type");
    }
  }
};

template <class T>
class mxArrayTo<T, typename std::enable_if<std::is_enum<T>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  T operator() (const mxArray* in) const {
    return static_cast<T>(mxArrayTo<typename std::underlying_type<T>::type>{}(in));
  }
};

template <class T, class A>
class mxArrayTo<typename std::vector<T,A>, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<A,  MexAllocator<T>>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  std::vector<T,A> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    if (mxIsComplex(in)) {
      throw std::invalid_argument(msg() + "Input is complex");
    }
    if (mxGetNumberOfElements(in) != 0 && mxGetData(in) == nullptr) {
      throw std::invalid_argument(msg() + "Input is invalid");
    }
    if (mxGetData(in) == nullptr && mxGetNumberOfElements(in) == 0) {
      return std::vector<T,A>();
    }
    std::vector<T,A> vec(mxGetNumberOfElements(in));
    if (mxIsCell(in)) {
      for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = mxArrayTo<T>{}(mxGetCell(in, i));
      }
    }
    else {
      copy(in, vec.begin());
    }
    return vec;
  }
  
  void copy(const mxArray* in, typename std::vector<T,A>::iterator begin) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    if (mxIsComplex(in)) {
      throw std::invalid_argument(msg() + "Input is complex");
    }
    if (mxGetData(in) == nullptr) {
      throw std::invalid_argument(msg() + "Input is invalid");
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
        throw std::invalid_argument(msg() + "Unknown input type");
        break;
    }
  }
};

template <class T>
class mxArrayTo<typename std::vector<T,  MexAllocator<T>>, typename std::enable_if<std::is_arithmetic<T>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  std::vector<T,  MexAllocator<T>> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    if (mxIsComplex(in)) {
      throw std::invalid_argument(msg() + "Input is complex");
    }
    if (mxGetNumberOfElements(in) != 0 && mxGetData(in) == nullptr) {
      throw std::invalid_argument(msg() + "Input is invalid");
    }
    if (mxGetData(in) == nullptr && mxGetNumberOfElements(in) == 0) {
      return std::vector<T, MexAllocator<T>>();
    }
    if (MexType<T>::ID::value != mxGetClassID(in)) {
      std::vector<T,  MexAllocator<T>> vec;
      vec.resize(mxGetNumberOfElements(in));
      mxArrayTo<std::vector<T>>{msg()}.copy(in, vec.begin());
      return vec;
    }
    else {
      std::vector<T,  MexAllocator<T>> vec(( MexAllocator<T>(in)));
      vec.resize(mxGetNumberOfElements(in));
      return vec;
    }
  }
};

template <class T>
class mxArrayTo<typename std::vector<typename std::vector<T>>, typename std::enable_if<std::is_arithmetic<T>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  std::vector<std::vector<T>> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    
    if (mxIsCell(in)) {
      std::vector<std::vector<T>> out(mxGetNumberOfElements(in));
      for (size_t i = 0; i < out.size(); ++i) {
        out[i] = mxArrayTo<std::vector<T>>{}(mxGetCell(in, i));
      }
      return out;
    }
    else {
      if (mxIsComplex(in)) {
        throw std::invalid_argument(msg() + "Input is complex");
      }
      if (mxGetNumberOfElements(in) != 0 && mxGetData(in) == nullptr) {
        throw std::invalid_argument(msg() + "Input is invalid");
      }
      if (mxGetData(in) == nullptr && mxGetNumberOfElements(in) == 0) {
        return std::vector<std::vector<T>>();
      }
      std::vector<std::vector<T>> out(mxGetM(in));
      void* data = mxGetData(in);
      for (size_t i = 0; i < out.size(); ++i) {
        out[i].resize(mxGetN(in));
        switch (mxGetClassID(in))
        {
          case mxLOGICAL_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<mxLogical*>(data)[j*out.size()+i];
            }
            break;
          case mxINT8_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<int8_t*>(data)[j*out.size()+i];
            }
            break;
          case mxUINT8_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<uint8_t*>(data)[j*out.size()+i];
            }
            break;
          case mxINT16_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<int16_t*>(data)[j*out.size()+i];
            }
            break;
          case mxUINT16_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<uint16_t*>(data)[j*out.size()+i];
            }
            break;
          case mxINT32_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<int32_t*>(data)[j*out.size()+i];
            }
            break;
          case mxUINT32_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<uint32_t*>(data)[j*out.size()+i];
            }
            break;
          case mxINT64_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<int64_t*>(data)[j*out.size()+i];
            }
            break;
          case mxUINT64_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<uint64_t*>(data)[j*out.size()+i];
            }
            break;
          case mxSINGLE_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<float*>(data)[j*out.size()+i];
            }
            break;
          case mxDOUBLE_CLASS:
            for (size_t j = 0; j < out[i].size(); ++j) {
              out[i][j] = reinterpret_cast<double*>(data)[j*out.size()+i];
            }
            break;
          default:
            throw std::invalid_argument(msg() + "Unknown input type");
            break;
        }
      }
      return out;
    }
  }
};

template <class T, class A>
class mxArrayTo<typename std::vector<T,A>, typename std::enable_if<!std::is_arithmetic<T>::value && !is_vector<T>::value>::type> : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  std::vector<T> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    if (!mxIsCell(in)) {
      throw std::invalid_argument(msg() + "Input is not a cell array");
    }
    std::vector<T,A> out(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = mxArrayTo<T>{}(mxGetCell(in, i));
    }
    return out;
  }
};

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
class mxArrayTo<typename std::unordered_map<Key,T,Hash,KeyEqual,Allocator>, void>  : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  std::unordered_map<Key,T,Hash,KeyEqual,Allocator> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null input");
    }
    std::unordered_map<Key,T,Hash,KeyEqual,Allocator> map;
    size_t size = mxGetNumberOfElements(in);
    for (size_t i = 0; i < size; ++i) {
      map.insert(std::make_pair(mxArrayTo<Key>{}(mxGetField(in, i, "key")), mxArrayTo<T>{}(mxGetField(in, i, "value"))));
    }
    return map;
  }
};

template <class T>
class mxArrayTo<MexHandle<T>,void>  : private ExceptionThrower
{
public:
  mxArrayTo(const std::string& msg = "") : ExceptionThrower(msg) {}
  mxArrayTo& operator() (const std::string& msg) {ExceptionThrower::operator() (msg); return *this;}

  MexHandle<T> operator() (const mxArray* in) const {
    if (in == nullptr) {
      throw std::invalid_argument(msg() + "Null object");
    }
    if (mxGetProperty(in, 0, "mexHandle_") == nullptr) {
      throw std::invalid_argument(msg() + "Invalid object");
    }
    if (mxGetData(mxGetProperty(in, 0, "mexHandle_")) == nullptr) {
      throw std::invalid_argument(msg() + "Invalid object");
    }
    T* ptr = reinterpret_cast<T*>(*((uint64_t *)mxGetData(mxGetProperty(in, 0, "mexHandle_"))));
    ptr = dynamic_cast<T*>(ptr);
    if (ptr == nullptr) {
      throw std::invalid_argument(msg() + "Null object");
    }
    MexHandle<T> handle;
    handle.set(ptr);
    return handle;
  }
};



template <class T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
mxArray* toMxArray(T in) {
  mxArray* out = mxCreateNumericMatrix(1,1, MexType<T>::ID::value, mxREAL);
  *static_cast<T*>(mxGetData(out)) = in;
  return out;
}

template <class T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
mxArray* toMxArray(T in) {
  return toMxArray(static_cast<uint32_t>(in));
}

template <class T, template <class> class A, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
mxArray* toMxArray(const std::vector<T, A<T>>& vec) {
  mxArray* out = mxCreateNumericMatrix(vec.size(), 1, MexType<T>::ID::value, mxREAL);
  std::copy(vec.begin(), vec.end(), static_cast<T*>(mxGetData(out)));
  return out;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
mxArray* toMxArray(const std::unordered_map<Key,T,Hash,KeyEqual,Allocator>& map) {
  const char* fieldnames[] = {"key", "value"};
  mxArray* out = mxCreateStructMatrix(1,map.size(), 2, fieldnames);
  size_t i = 0;
  for (auto it = map.begin(); it != map.end(); ++it, ++i) {
    mxSetField(out, i, fieldnames[0], toMxArray(it->first));
    mxSetField(out, i, fieldnames[1], toMxArray(it->second));
  }
  return out;
}

template <class T, template <class> class A, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
mxArray* toMxArray(const std::vector<T, A<T>>& vec) {
  mxArray* out = mxCreateCellMatrix(vec.size(), 1);
  for (size_t i = 0; i < vec.size(); ++i) {
    mxSetCell(out, i, toMxArray(vec[i]));
  }
  return out;
}

template <class T>
mxArray* toMxArray(const std::vector<T,  MexAllocator<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(0, 0, MexType<T>::ID::value, mxREAL);
  mxSetData(out, all.ptr());
  mxSetM(out, all.size());
  mxSetN(out, 1);
  return out;
}

template <typename T>
mxArray* toMxArray(MexHandle<T>&& u) {
  mxArray* out = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  *((uint64_t *)mxGetData(out)) = reinterpret_cast<uint64_t>(u.get());
  return out;
}

#endif
