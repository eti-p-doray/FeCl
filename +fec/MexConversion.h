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
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <mex.h>

#include "MexAllocator.h"

template <typename T> mxClassID mexClassId() {return mxUNKNOWN_CLASS;}
template <> mxClassID mexClassId<double>() {return mxDOUBLE_CLASS;}
template <> mxClassID mexClassId<float>() {return mxSINGLE_CLASS;}
template <> mxClassID mexClassId<bool>() {return mxLOGICAL_CLASS;}
template <> mxClassID mexClassId<int8_t>() {return mxINT8_CLASS;}
template <> mxClassID mexClassId<uint8_t>() {return mxUINT8_CLASS;}
template <> mxClassID mexClassId<int16_t>() {return mxINT16_CLASS;}
template <> mxClassID mexClassId<uint16_t>() {return mxUINT16_CLASS;}
template <> mxClassID mexClassId<int32_t>() {return mxINT32_CLASS;}
template <> mxClassID mexClassId<uint32_t>() {return mxUINT32_CLASS;}
template <> mxClassID mexClassId<int64_t>() {return mxINT64_CLASS;}
template <> mxClassID mexClassId<uint64_t>() {return mxUINT64_CLASS;}


template <class T, class Enable = void>
class mxArrayTo {
public:
  static T f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetNumberOfElements(in) != 1 || mxGetData(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    switch (mxGetClassID(in))
    {
      case mxINT8_CLASS:
        return *reinterpret_cast<int8_t*>(mxGetData(in));
        break;
        
      case mxUINT8_CLASS:
        return *reinterpret_cast<uint8_t*>(mxGetData(in));
        break;
        
      case mxINT16_CLASS:
        return *reinterpret_cast<int16_t*>(mxGetData(in));
        break;
        
      case mxUINT16_CLASS:
        return *reinterpret_cast<uint16_t*>(mxGetData(in));
        break;
        
      case mxINT32_CLASS:
        return *reinterpret_cast<int32_t*>(mxGetData(in));
        break;
        
      case mxUINT32_CLASS:
        return *reinterpret_cast<uint32_t*>(mxGetData(in));
        break;
        
      case mxINT64_CLASS:
        return *reinterpret_cast<int64_t*>(mxGetData(in));
        break;
        
      case mxUINT64_CLASS:
        return *reinterpret_cast<uint64_t*>(mxGetData(in));
        break;
        
      case mxSINGLE_CLASS:
        return *reinterpret_cast<float*>(mxGetData(in));
        break;
        
      case mxDOUBLE_CLASS:
        return *reinterpret_cast<double*>(mxGetData(in));
        break;
        
      default:
        throw std::invalid_argument("Unknown class");
        break;
    }
  }
};

template <class T>
mxArray* toMxArray(const T& in) {
  mxArray* out = mxCreateDoubleScalar(in);
  return out;
}

template <class T, class Enable = void>
class mxCellArrayTo {
public:
  static std::vector<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (!mxIsCell(in)) {
      throw std::invalid_argument("Invalid data");
    }
    
    std::vector<T> out(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = mxArrayTo<T>::f(mxGetCell(in, i));
    }
    return out;
  }
};

template <typename T>
class mxCellArrayTo<T, typename std::enable_if<std::is_enum<T>::value>::type> {
public:
  static std::vector<T> f(const mxArray* in, const char* const enumeration[], size_t count) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (!mxIsCell(in)) {
      throw std::invalid_argument("Invalid data");
    }
    
    std::vector<T> out(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = mxArrayTo<T>::f(mxGetCell(in, i), enumeration, count);
    }
    return out;
  }
};

template <class T, class A, template <class, class> class V>
class mxArrayTo<V<T,A>> {
public:
  static V<T,A> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    V<T,A> vec(mxGetNumberOfElements(in));
    switch (mxGetClassID(in))
    {
      case mxINT8_CLASS:
        std::copy(reinterpret_cast<int8_t*>(mxGetData(in)), reinterpret_cast<int8_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT8_CLASS:
        std::copy(reinterpret_cast<uint8_t*>(mxGetData(in)), reinterpret_cast<uint8_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT16_CLASS:
        std::copy(reinterpret_cast<int16_t*>(mxGetData(in)), reinterpret_cast<int16_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT16_CLASS:
        std::copy(reinterpret_cast<uint16_t*>(mxGetData(in)), reinterpret_cast<uint16_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT32_CLASS:
        std::copy(reinterpret_cast<int32_t*>(mxGetData(in)), reinterpret_cast<int32_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT32_CLASS:
        std::copy(reinterpret_cast<uint32_t*>(mxGetData(in)), reinterpret_cast<uint32_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT64_CLASS:
        std::copy(reinterpret_cast<int64_t*>(mxGetData(in)), reinterpret_cast<int64_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT64_CLASS:
        std::copy(reinterpret_cast<uint64_t*>(mxGetData(in)), reinterpret_cast<uint64_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxSINGLE_CLASS:
        std::copy(reinterpret_cast<float*>(mxGetData(in)), reinterpret_cast<float*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxDOUBLE_CLASS:
        std::copy(reinterpret_cast<double*>(mxGetData(in)), reinterpret_cast<double*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      default:
        throw std::invalid_argument("Unknown class");
        break;
    }
    return vec;
  }
};

template <class T, template <class, class> class V>
class mxArrayTo<V<T, MexAllocator<T>>> {
public:
  static V<T, MexAllocator<T>> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr || mexClassId<T>() != mxGetClassID(in)) {
      throw std::invalid_argument("Unknown class");
    }
    V<T, MexAllocator<T>> vec((MexAllocator<T>(in)));
    vec.resize(mxGetNumberOfElements(in));
    return vec;
  }
};

template <class T, template <class> class A, template <class, class> class V>
mxArray* toMxArray(const V<T, A<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(vec.size(), 1, mexClassId<T>(), mxREAL);
  std::copy(vec.begin(), vec.end(), static_cast<T*>(mxGetData(out)));
  
  return out;
}

template <class T, template <class, class> class V>
mxArray* toMxArray(const V<T, MexAllocator<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(0, 0, mexClassId<T>(), mxREAL);
  mxSetData(out, all.ptr());
  mxSetM(out, all.size());
  mxSetN(out, 1);
  
  return out;
}

template <class T>
class mxArrayTo<std::unique_ptr<T>> {
public:
  static std::unique_ptr<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxGetProperty(in, 0, "mexHandle_") == nullptr) {
      throw std::invalid_argument("wrong arg class");
    }
    if (mxGetData(mxGetProperty(in, 0, "mexHandle_")) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    
    
    T* ptr = reinterpret_cast<T*>(*((uint64_t *)mxGetData(mxGetProperty(in, 0, "mexHandle_"))));
    ptr = dynamic_cast<T*>(ptr);
    
    if (boost::serialization::extended_type_info_no_rtti<T>::get_const_instance().get_derived_extended_type_info(*ptr) == nullptr) {
      //throw std::invalid_argument("Invalid class");
    }
    
    if (ptr == nullptr) {
      throw std::invalid_argument("null ptr received");
    }
    return std::unique_ptr<T>(ptr);
  }
};

template <class T>
mxArray* toMxArray(std::unique_ptr<T>&& u) {
  mxArray* out = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  *((uint64_t *)mxGetData(out)) = reinterpret_cast<uint64_t>(u.release());
  
  return out;
}


template <typename T>
class mxArrayTo<T, typename std::enable_if<std::is_enum<T>::value>::type> {
public:
  static T f(const mxArray* in, const char* const enumeration[], size_t count) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxArrayToString(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    for (size_t i = 0; i < count; i++) {
      if (!strcmp(enumeration[i], mxArrayToString(in))) {
        return static_cast<T>(i);
      }
    }
    throw std::invalid_argument("Unknown enum");
    return static_cast<T>(0);
  }
};


class sink_counter : public boost::iostreams::sink {
public:
  sink_counter() = default;
  sink_counter(const sink_counter&) = default;
  sink_counter(size_t& count) {count_ = &count; *count_ = 0;}
  
  std::streamsize write(const char* s, std::streamsize n) {*count_ += n; return n;}
  size_t size() const {return *count_;}
private:
  size_t* count_ = nullptr;
};

const char* saveStructFieldNames[] = {"data"};

template <class ... Childs>
struct RegisterAgent {
};

template <class Child, class ... Childs>
struct RegisterAgent<Child, Childs...> : RegisterAgent<Childs...> {
  template <typename Archive>
  void register_type(Archive& archive) const
  {
    archive.template register_type<Child>();
    RegisterAgent<Childs...>::register_type(archive);
  }
};

template <>
struct RegisterAgent<> {
  template <typename Archive>
  void register_type(Archive& archive) const {}
};

template <class T, class RegisterAgent>
mxArray* save(const std::unique_ptr<T>& u, RegisterAgent registerAgent)
{
  mxArray* save = mxCreateStructMatrix(1, 1, 1, {saveStructFieldNames});
  const T* base_pointer = u.get();
  
  size_t serialSize = 0;;
  sink_counter countSr(serialSize);
  boost::iostreams::stream< sink_counter > countSource(countSr);
  boost::archive::binary_oarchive countOa(countSource);
  registerAgent.register_type(countOa);
  
  countOa & BOOST_SERIALIZATION_NVP(base_pointer);
  
  mxArray* data = mxCreateNumericMatrix(serialSize + 4096, 1, mexClassId<uint8_t>(), mxREAL);
  
  boost::iostreams::basic_array_sink<char> sr(reinterpret_cast<char*>(mxGetData(data)), serialSize + 4096);
  boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > source(sr);
  boost::archive::binary_oarchive oa(source);
  registerAgent.register_type(oa);
  oa & BOOST_SERIALIZATION_NVP(base_pointer);
  
  mxSetField(save, 0, "data", data);
  
  return save;
}

template <class T, class RegisterAgent>
std::unique_ptr<T> load(const mxArray* in, RegisterAgent registerAgent)
{
  mxArray* data = mxGetField(in, 0, "data");
  if (data == nullptr) {
    std::invalid_argument("Null mxArray");
  }
  if (mxGetData(data) == nullptr) {
    std::invalid_argument("Invalid data");
  }
  size_t serialSize = mxGetNumberOfElements(data);
  
  boost::iostreams::basic_array_source<char> sr(reinterpret_cast<char*>(mxGetData(data)), serialSize);
  boost::iostreams::stream< boost::iostreams::basic_array_source<char> > source(sr);
  boost::archive::binary_iarchive ia(source);
  registerAgent.register_type(ia);
  
  std::unique_ptr<T> u(nullptr);
  T* base_ptr;
  ia & BOOST_SERIALIZATION_NVP(base_ptr);
  u.reset(base_ptr);
  return u;
}

#endif
