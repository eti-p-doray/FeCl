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

#ifndef FEC_SERIALIZATION_H
#define FEC_SERIALIZATION_H

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
  
  size_t serialSize = 0;
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
