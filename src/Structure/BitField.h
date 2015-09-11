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
 
 Declaration of BitField class
 ******************************************************************************/

#ifndef BITFIELD_H
#define BITFIELD_H

#include <iostream>
#include <stdint.h>
#include <vector>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

namespace fec {

/**
 *  This class emulates a reference to a single bit.
 */
class BitReference {
public:
  BitReference(uint8_t* bloc, uint8_t index) {bloc_ = bloc; index_ = index;}
  
  BitReference& operator= (BitReference x) {(*this) = bool(x); return *this;}/**< Assignement operator. */
  BitReference& operator= (bool x) {(*this) &= 0; (*this) |= x; return *this;}/**< Assignement operator. */
  inline operator bool () const {return ((*bloc_)>>index_) & 1;}/**< Converts the object to a value. */
  
  BitReference& operator&= (bool x) {*bloc_ &= ~(uint8_t(!x)<<index_); return *this;} /**< Bitwise AND assignement. */
  BitReference& operator|= (bool x) {*bloc_ |= (uint8_t(x)<<index_); return *this;}; /**< Bitwise OR assignement. */
  BitReference& operator^= (bool x) {*bloc_ ^= (uint8_t(x)<<index_); return *this;}; /**< Bitwise XOR assignement. */
  
private:
  uint8_t* bloc_;
  uint8_t index_;
};

/**
 *  This class represents a field 
 *  in which every bits can be accessed individualy.
 *  A Bitfield has a fixed size defined by its holder type 
 *  and can easily be converted back and forth to this type.
 *  \tparam T Data holder type
 */
template <typename T>
class BitField {
  friend class boost::serialization::access;
  typedef uint8_t BlocType;
public:
  BitField() = default;
  BitField(T value) {value_ = value;}
  BitField(const BitField& b) = default;
  
  T operator=(T value) {value_ = value; return value_;}
  
  inline operator T() const {return value_;}
  
  inline BitReference operator[](size_t i) {return BitReference(&at<uint8_t>(i/8), i%8);}
  inline bool operator[](size_t i) const {return (value_>>i) & 1;}
  inline bool test(size_t i) const {return value_ & (size_t(1)<<i);}
  inline void set(size_t i) {value_ |= (size_t(1)<<i);}
  inline void set(size_t i, bool val) {value_ &= ~(size_t(1)<<i); value_ |= size_t(val) << i;}
  inline void clear(size_t i) {value_ &= ~(size_t(1)<<i);}
  inline void toggle(size_t i) {value_ ^= (size_t(1)<<i);}
  
  BitField operator~();
  BitField operator<<(int b) {return value_ << b;}
  BitField operator>>(int b) {return value_ >> b;}
  
  template <typename S> void operator+=(S b) {value_ += b;}
  template <typename S> void operator-=(S b) {value_ -= b;}
  template <typename S> void operator*=(S b) {value_ *= b;}
  template <typename S> void operator/=(S b) {value_ /= b;}
  template <typename S> void operator%=(S b) {value_ %= b;}
  template <typename S> void operator&=(S b) {value_ &= b;}
  template <typename S> void operator|=(S b) {value_ |= b;}
  template <typename S> void operator^=(S b) {value_ ^= b;}
  template <typename S> void operator<<=(S b) {value_ <<= b;}
  template <typename S> void operator>>=(S b) {value_ >>= b;}
  
  void operator++() {++value_;}
  void operator--() {--value_;}
  void operator++(int) {value_++;}
  void operator--(int) {value_--;}
  
  template <typename S = BlocType>
  /**
   *  Field size in bloc type
   *  \tparam S Bloc type
   */
  static uint8_t blocSize() {return sizeof(S)/sizeof(uint8_t) * 8;}
  static uint8_t size() {return sizeof(T)/sizeof(uint8_t) * 8;} /**< Field size in bits */
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_NVP(value_);
  }
  
  template <typename S = BlocType>       S& at(uint64_t i) {return data<S>()[i];}
  template <typename S = BlocType> const S& at(uint64_t i) const {return data<S>()[i];}
  template <typename S = BlocType>       S* data() {return reinterpret_cast<S*>(&value_);}
  template <typename S = BlocType> const S* data() const  {return reinterpret_cast<const S*>(&value_);}
  
  T value_;
};
  
}

template <typename T>
bool parity(fec::BitField<T> a) {
  bool x = false;
  for (int i = 0; i < a.size(); ++i) {
    x ^= a[i];
  }
  return x;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const fec::BitField<T>& a)
{
  for (uint64_t i = 0; i < 4; ++i) {
    os << a.test(i);
  }
  return os;
}

template <typename T, typename S> fec::BitField<T> operator+(fec::BitField<T> a, S b) {a += b; return a;}
template <typename T, typename S> fec::BitField<T> operator-(fec::BitField<T> a, S b) {a -= b; return a;}
template <typename T, typename S> fec::BitField<T> operator*(fec::BitField<T> a, S b) {a *= b; return a;}
template <typename T, typename S> fec::BitField<T> operator/(fec::BitField<T> a, S b) {a /= b; return a;}
template <typename T, typename S> fec::BitField<T> operator%(fec::BitField<T> a, S b) {a %= b; return a;}
template <typename T, typename S> fec::BitField<T> operator&(fec::BitField<T> a, S b) {a &= b; return a;}
template <typename T, typename S> fec::BitField<T> operator|(fec::BitField<T> a, S b) {a |= b; return a;}
template <typename T, typename S> fec::BitField<T> operator^(fec::BitField<T> a, S b) {a ^= b; return a;}
template <typename T, typename S> fec::BitField<T> operator<<(fec::BitField<T> a, S b) {a <<= b; return a;}
template <typename T, typename S> fec::BitField<T> operator>>(fec::BitField<T> a, S b) {a >>= b; return a;}


#endif
