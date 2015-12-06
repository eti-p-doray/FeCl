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

#ifndef FEC_MULTIITERATOR_H
#define FEC_MULTIITERATOR_H

#include <array>
#include <tuple>

#include "../BitField.h"
#include "../DecoderAlgorithm.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
  
    /**
     *  This class is an iterator on the codec data flow.
     */
    template <class Iterator, class Key, Key... keys>
    class MultiIterator {
      struct Value {
        Iterator iterator;
        size_t increment = 0;
        bool has = false;
      };
    public:
      using difference_type = std::ptrdiff_t; //almost always ptrdif_t
      using value_type = MultiIterator; //almost always T
      using reference = MultiIterator&; //almost always T& or const T&
      using pointer = MultiIterator*; //almost always T* or const T*
      using iterator_category = typename Iterator::iterator_category;
      
      MultiIterator() = default;
      MultiIterator(std::initializer_list<std::tuple<Key, Iterator, size_t>> l);
      
      void insert(Key key, Iterator it, size_t increment);
      const Iterator at(Key key) const;
      bool count(Key key) const;
      
      inline MultiIterator& operator ++ ();
      inline MultiIterator operator ++ (int) {auto tmp = *this; ++*this; return tmp;}
      inline MultiIterator& operator -- ();
      inline MultiIterator operator -- (int) {auto tmp = *this; ++*this; return tmp;}
      inline MultiIterator& operator += (size_t x);
      inline MultiIterator& operator -= (size_t x);
      inline MultiIterator operator + (size_t x) const {auto tmp = *this; tmp+=x; return tmp;}
      inline MultiIterator operator - (size_t x) const {auto tmp = *this; tmp-=x; return tmp;}
      inline std::ptrdiff_t operator - (const MultiIterator& b) const;
      inline bool operator == (const MultiIterator& b) const;
      inline bool operator != (const MultiIterator& b) const {return !(*this == b);}
      inline bool operator < (const MultiIterator& b) const;
      inline bool operator <= (const MultiIterator& b) const;
      inline bool operator > (const MultiIterator& b) const {return !(*this <= b);}
      inline bool operator >= (const MultiIterator& b) const {return !(*this < b);}

      inline MultiIterator& operator [] (size_t i);
      inline MultiIterator* operator -> ();
      
    private:
      std::array<Value,sizeof...(keys)>  map_;
    };
    
  }
  
}

template <class Iterator, class Key, Key... keys>
fec::detail::MultiIterator<Iterator,Key,keys...>::MultiIterator(std::initializer_list<std::tuple<Key, Iterator, size_t>> l)
{
  for (auto i : l) {
    map_[std::get<0>(i)].iterator = std::get<1>(i);
    map_[std::get<0>(i)].increment = std::get<2>(i);
    map_[std::get<0>(i)].has = true;
  }
}

template <class Iterator, class Key, Key... keys>
void fec::detail::MultiIterator<Iterator,Key,keys...>::insert(Key key, Iterator it, size_t increment)
{
  map_[key].iterator = it;
  map_[key].increment = increment;
  map_[key].has = true;
}

template <class Iterator, class Key, Key... keys>
const Iterator fec::detail::MultiIterator<Iterator,Key,keys...>::at(Key key) const
{
  return map_[key].iterator;
}

template <class Iterator, class Key, Key... keys>
bool fec::detail::MultiIterator<Iterator,Key,keys...>::count(Key key) const
{
  return map_[key].has;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>& fec::detail::MultiIterator<Iterator,Key,keys...>::operator ++ ()
{
  for (auto & i : map_) {
    i.iterator += i.increment;
  }
  return *this;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>& fec::detail::MultiIterator<Iterator,Key,keys...>::operator -- ()
{
  for (auto & i : map_) {
    i.iterator -= i.increment;
  }
  return *this;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>& fec::detail::MultiIterator<Iterator,Key,keys...>::operator += (size_t x)
{
  for (auto & i : map_) {
    i.iterator += i.increment*x;
  }
  return *this;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>& fec::detail::MultiIterator<Iterator,Key,keys...>::operator -= (size_t x)
{
  for (auto & i : map_) {
    i.iterator -= i.increment * x;
  }
  return *this;
}

template <class Iterator, class Key, Key... keys>
inline std::ptrdiff_t fec::detail::MultiIterator<Iterator,Key,keys...>::operator - (const MultiIterator& b) const
{
  for (auto i = map_.begin(), j = b.map_.begin(); i < map_.end(); ++i, ++j) {
    if (i->has && j->has) {
      return (i->iterator - j->iterator)/i->increment;
    }
  }
  return 0;
}

template <class Iterator, class Key, Key... keys>
inline bool fec::detail::MultiIterator<Iterator,Key,keys...>::operator == (const MultiIterator& b) const
{
  for (auto i = map_.begin(), j = b.map_.begin(); i < map_.end(); ++i, ++j) {
    if (i->has != j->has) return false;
    if (i->has) {
      if (i->iterator != j->iterator) return false;
    }
  }
  return true;
}

template <class Iterator, class Key, Key... keys>
inline bool fec::detail::MultiIterator<Iterator,Key,keys...>::operator < (const MultiIterator& b) const
{
  for (auto i = map_.begin(), j = b.map_.begin(); i < map_.end(); ++i, ++j) {
    if (i->has && j->has) {
      if (i.iterator != j.iterator) {
        return i.iterator < j.iterator;
      }
    }
  }
  return false;
}

template <class Iterator, class Key, Key... keys>
inline bool fec::detail::MultiIterator<Iterator,Key,keys...>::operator <= (const MultiIterator& b) const
{
  for (auto i = map_.begin(), j = b.map_.begin(); i < map_.end(); ++i, ++j) {
    if (i->has && j->has) {
      if (i.iterator != j.iterator) {
        return i.iterator < j.iterator;
      }
    }
  }
  return true;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>& fec::detail::MultiIterator<Iterator,Key,keys...>::operator [] (size_t x)
{
  return *this + x;
}

template <class Iterator, class Key, Key... keys>
inline fec::detail::MultiIterator<Iterator,Key,keys...>* fec::detail::MultiIterator<Iterator,Key,keys...>::operator -> ()
{
  return this;
}

#endif