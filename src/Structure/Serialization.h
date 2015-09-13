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
 
 Declaration of Trellis class
 ******************************************************************************/

#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <assert.h>
#include <iostream>
#include <vector>
#include <memory>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

template <class ... Childs>
struct DerivedTypeHolder {
};

template <class Child, class ... Childs>
struct DerivedTypeHolder<Child, Childs...> : DerivedTypeHolder<Childs...> {
  template <typename Archive>
  void register_type(Archive& archive) const
  {
    archive.template register_type<Child>();
    DerivedTypeHolder<Childs...>::register_type(archive);
  }
  void register_type() const
  {
    boost::serialization::type_info_implementation<Child>::type::get_const_instance();
    DerivedTypeHolder<Childs...>::register_type();
  }
};

template <>
struct DerivedTypeHolder<> {
  template <typename Archive>
  void register_type(Archive& archive) const {}
  void register_type() const {}
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

template <class T, class DerivedTypeHolder>
size_t archiveSize(const T* u, DerivedTypeHolder derived)
{
  const T* base_pointer = u;
  
  size_t serialSize = 0;;
  sink_counter countSr(serialSize);
  boost::iostreams::stream< sink_counter > countSource(countSr);
  boost::archive::binary_oarchive countOa(countSource);
  derived.register_type(countOa);
  
  countOa & BOOST_SERIALIZATION_NVP(base_pointer);
  
  return serialSize + 4096;
}

template <class T, class DerivedTypeHolder>
void save(const T* u, char* buffer, size_t size, DerivedTypeHolder derived) {
  const T* base_pointer = u;
  
  boost::iostreams::basic_array_sink<char> sr(buffer, size);
  boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > source(sr);
  boost::archive::binary_oarchive oa(source);
  derived.register_type(oa);
  oa & BOOST_SERIALIZATION_NVP(base_pointer);
}

template <class T, class DerivedTypeHolder>
std::unique_ptr<T> load(const char* buffer, size_t size, DerivedTypeHolder derived)
{
  boost::iostreams::basic_array_source<char> sr(buffer, size);
  boost::iostreams::stream< boost::iostreams::basic_array_source<char> > source(sr);
  boost::archive::binary_iarchive ia(source);
  derived.register_type(ia);
  
  std::unique_ptr<T> u(nullptr);
  T* base_ptr;
  ia & BOOST_SERIALIZATION_NVP(base_ptr);
  u.reset(base_ptr);
  return u;
}

#endif
