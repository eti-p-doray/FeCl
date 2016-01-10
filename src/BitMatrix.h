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

#ifndef FEC_BIT_MATRIX_H
#define FEC_BIT_MATRIX_H

#include <vector>
#include <algorithm>
#include <type_traits>
#include <iterator>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

#include "BitField.h"

namespace fec {
  
template <class T> class BitMatrix;
template <class T> class SparseBitMatrix;
  
struct Empty {};

/**
 *  This class represents a sparse bit matrix.
 *  Only non-zero elements are stored.
 *  The class defines basic methods to manipulate the matrix such as row/column swapping.
 */
template <class T>
class SparseBitMatrix
{
  friend class boost::serialization::access;

  struct RowIdx {
    size_t begin;
    size_t end;
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
  };
  
  template <bool isConst> class IteratorImpl;
  
public:
  
  template <bool isConst, bool isTranslated = false, bool isScaled = false>
  class RowRef {
    friend IteratorImpl<isConst>;
    friend class SparseBitMatrix;
  public:
    using const_iterator = typename std::vector<std::pair<size_t,BitField<T>>>::const_iterator;
    using iterator = typename std::conditional<isConst,const_iterator,typename std::vector<std::pair<size_t,BitField<T>>>::iterator>::type;
    
    RowRef(const RowRef&) = default;

    template <bool isConst2, bool isTranslated2, bool isScaled2> inline void operator = (RowRef<isConst2,isTranslated2,isScaled2> b);
    
    inline const_iterator cbegin() const {return begin_ + rowIdx_.begin;}
    inline const_iterator cend() const {return begin_ + rowIdx_.end;}
    inline iterator begin() const {return begin_ + rowIdx_.begin;}
    inline iterator end() const {return begin_ + rowIdx_.end;}
    
    inline BitField<T> test(size_t j) const {auto it = std::lower_bound(begin(), end(), j); return (it != end() && it->first == j) ? it->second : 0;}
    inline size_t first() const {return end() - begin() == 0 ? -1 : begin()->first + translation();}
    inline size_t size() const {return rowIdx_.end - rowIdx_.begin;}
    inline size_t size(const std::array<size_t,2>& range) const {return std::lower_bound(begin(), end(), range[1]-translation(), comp_) - std::lower_bound(begin(), end(), range[0]-translation(), comp_);}
    inline bool empty() const {return (end() - begin()) == 0;} /**< \return True if the row contains no non-zero elements. */
    
    inline void set(size_t j, BitField<T> val = 1) const;
    inline void clear(size_t j) const;
    
    inline void swap(size_t a, size_t b);
    inline void move(size_t a, size_t b);
    
    inline RowRef<true, true, isScaled> operator() (const std::array<size_t,2> colRange) const;

    void swap(RowRef& b) {std::swap(rowIdx_, b.rowIdx_);}
    friend void swap(RowRef& a, RowRef& b) {a.swap(b);}
    
    inline RowRef& operator *= (int b);
    inline RowRef<true, isTranslated, true> operator * (int b) const;
    
    template <bool U = isTranslated, typename std::enable_if<U>::type* = nullptr> size_t translation() const {return translation_;}
    template <bool U = isTranslated, typename std::enable_if<!U>::type* = nullptr> size_t translation() const {return 0;}
    
    template <bool U = isScaled, typename std::enable_if<U>::type* = nullptr> int scale() const {return scale_;}
    template <bool U = isScaled, typename std::enable_if<!U>::type* = nullptr> int scale() const {return 0;}
    
  private:
    inline RowRef() = default;
    inline RowRef(iterator begin, typename std::conditional<isConst,RowIdx,RowIdx&>::type rowIdx) : begin_(begin), rowIdx_(rowIdx) {}
    
    iterator begin_;
    typename std::conditional<isConst,RowIdx,RowIdx&>::type rowIdx_;
    typename std::conditional<isTranslated,size_t,Empty> translation_;
    typename std::conditional<isScaled,int,Empty> scale_;
  };
  
  template <bool isConst>
  class RowPtr {
  public:
    inline RowPtr(const RowRef<isConst,false>& row) : row_(row) {}
    inline RowRef<isConst,false>* operator-> () {return &row_;}
    
  private:
    RowRef<isConst,false> row_;
  };
  
private:
  template <bool isConst>
  class IteratorImpl {
    friend class SparseBitMatrix;
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = RowRef<true>;
    using reference = RowRef<false>;
    using pointer = RowPtr<isConst>;
    using iterator_category = std::random_access_iterator_tag;
    
    IteratorImpl(const IteratorImpl&) = default;
    
    inline IteratorImpl& operator++() {++rowIdx_; return *this;}
    inline IteratorImpl& operator--() {--rowIdx_; return *this;}
    inline IteratorImpl operator++(int) {auto tmp = *this; ++rowIdx_; return tmp;}
    inline IteratorImpl operator--(int) {auto tmp = *this; --rowIdx_; return tmp;}
    
    inline IteratorImpl& operator+=(size_t x) {rowIdx_ += x; return *this;}
    inline IteratorImpl& operator-=(size_t x) {rowIdx_ -= x; return *this;}
    
    inline bool operator<(IteratorImpl b) const {return rowIdx_ < b.rowIdx_;}
    inline bool operator<=(IteratorImpl b) const {return rowIdx_ <= b.rowIdx_;}
    inline bool operator>(IteratorImpl b) const {return rowIdx_ > b.rowIdx_;}
    inline bool operator>=(IteratorImpl b) const {return rowIdx_ >= b.rowIdx_;}
    inline bool operator==(IteratorImpl b) const {return rowIdx_ == b.rowIdx_;}
    inline bool operator!=(IteratorImpl b) const {return rowIdx_ != b.rowIdx_;}
    
    template <typename S> friend IteratorImpl operator+(const IteratorImpl& a, S b) {return IteratorImpl(a.elem_, a.rowIdx_+b);}
    template <typename S> friend IteratorImpl operator+(S a, const IteratorImpl& b) {return IteratorImpl(b.elem_, b.rowIdx_+a);}
    
    template <typename S> friend IteratorImpl operator-(const IteratorImpl& a, S b)  {return IteratorImpl(a.elem_, a.rowIdx_-b);}
    inline friend size_t operator-(const IteratorImpl& a, const IteratorImpl& b) {return a.rowIdx_ - b.rowIdx_;}
    
    inline RowRef<isConst> operator*() {return RowRef<isConst>(elem_, *rowIdx_);}
    inline RowPtr<isConst> operator-> () {return RowPtr<isConst>(*(*this));}
    inline RowRef<isConst> operator[] (size_t i) {return RowRef<isConst>(elem_, rowIdx_[i]);}
    
  private:
    inline IteratorImpl(typename std::conditional<isConst,typename std::vector<std::pair<size_t,BitField<T>>>::const_iterator,typename std::vector<std::pair<size_t,BitField<T>>>::iterator>::type elem, typename std::conditional<isConst,typename std::vector<RowIdx>::const_iterator,typename std::vector<RowIdx>::iterator>::type rowIdx) : elem_(elem), rowIdx_(rowIdx) {}
    
    typename std::conditional<isConst,typename std::vector<std::pair<size_t,BitField<T>>>::const_iterator,typename std::vector<std::pair<size_t,BitField<T>>>::iterator>::type elem_;
    typename std::conditional<isConst,typename std::vector<RowIdx>::const_iterator,typename std::vector<RowIdx>::iterator>::type rowIdx_;
  };
  
public:
  
  using const_iterator = IteratorImpl<true>;
  using iterator = IteratorImpl<false>;
  
  SparseBitMatrix() = default;
  SparseBitMatrix(const BitMatrix<T>& b) {*this = b;} /**< Copy constructor. */
  inline SparseBitMatrix(size_t rows, size_t cols, size_t rowSizes, size_t width = 1) {resize(rows, cols, rowSizes, width);}
  inline SparseBitMatrix(const std::vector<size_t>& rowSizes, size_t cols, size_t width = 1) {resize(rowSizes, cols, width);}

  inline SparseBitMatrix& operator = (const BitMatrix<T>& b);
  
  inline void resize(size_t rows, size_t cols, size_t rowSizes, size_t width = 1);
  inline void resize(const std::vector<size_t>& rowSizes, size_t cols, size_t width = 1);
  
  inline size_t rows() const {return rowIdx_.size();} /**< Access the number of rows in the matrix. */
  inline size_t cols() const {return cols_;} /**< Access the number of columns in the matrix. */
  inline size_t width() const {return width_;} /**< Access the matrix width. */
  inline size_t size() const {return elementIdx_.size();} /**< Access the number of stored elements. */

  inline SparseBitMatrix operator() (const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange) const;
  
  inline iterator begin() {return iterator(elementIdx_.begin(), rowIdx_.begin());}
  inline iterator end() {return iterator(elementIdx_.begin(), rowIdx_.end());}
  inline const_iterator cbegin() const {return const_iterator(elementIdx_.cbegin(), rowIdx_.begin());}
  inline const_iterator cend() const {return const_iterator(elementIdx_.cbegin(), rowIdx_.cend());}
  inline const_iterator begin() const {return cbegin();}
  inline const_iterator end() const {return cend();}
  
  inline RowRef<true> operator[] (size_t i) const {return RowRef<true>(elementIdx_.begin(), rowIdx_[i]);}
  inline RowRef<false> operator[] (size_t i) {return RowRef<false>(elementIdx_.begin(), rowIdx_[i]);}
  inline std::pair<size_t,BitField<T>> at(size_t i) const {return elementIdx_[i];}
  
  inline void colSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const;
  inline void colSizes(std::vector<size_t>& dst) const {colSizes({0, rows()}, {0, cols()}, dst);}
  inline std::vector<size_t> colSizes() const {std::vector<size_t> x(rows()); colSizes({0, rows()}, {0, cols()}, x); return x;}
  
  inline void rowSizes(std::vector<size_t>& x) const {rowSizes({0, rows()}, {0, cols()}, x);}
  inline std::vector<size_t> rowSizes() const {std::vector<size_t> x(rows()); rowSizes({0, rows()}, {0, cols()}, x); return x;}
  inline void rowSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const;
  
  inline void moveCol(size_t a, size_t b);
  
  inline void swapCols(size_t a, size_t b) {swapCols(a, b, {0, rows()});}
  inline void swapCols(size_t a, size_t b, const std::array<size_t,2>& rowRange);

  inline SparseBitMatrix transpose() const;
  inline SparseBitMatrix shrink() const;
  
  inline size_t isALT();
  inline size_t makeALT();
  
private:
  template <typename Archive> void serialize(Archive & ar, const unsigned int version);
  
  class Comp {public: bool operator () (const std::pair<size_t,BitField<T>>& a, size_t b) const {return a.first < b;}};
  static const Comp comp_;
  
  size_t width_;
  size_t cols_ = 0;
  std::vector<std::pair<size_t,BitField<T>>> elementIdx_;
  std::vector<RowIdx> rowIdx_;
};

/**
 *  This class represents a full bit matrix.
 *  Every bit is stored in a compact BitField.
 */
template <class T>
class BitMatrix
{
  friend class boost::serialization::access;
public:
  /**
   *  This class is a full bit row.
   */
  class Row
  {
    friend class boost::serialization::access;
    friend class BitMatrix;

    /**
     *  This is a random access input iterator of a BitMatrix::Row.
     *  It iterates over every elements in a row.
     */
    class IteratorImpl {
      friend class Row;
    public:
      using difference_type = std::ptrdiff_t;
      using value_type = BitField<T>;
      using reference = BitField<T>&;
      using pointer = BitField<T>*;
      using iterator_category = std::random_access_iterator_tag;
      
      inline void operator++() {idx_ += align_; if (idx_ == blocSize()) {++bloc_; idx_ = 0;}}
      inline void operator--() {if (idx_ == 0) {--bloc_; idx_ = blocSize()-align_;} else {idx_ -= align_;}}
      
      inline void operator+=(size_t x) {idx_ += x * align_; bloc_ += idx_ / blocSize(); idx_ %= blocSize();}
      inline void operator-=(size_t x) {idx_ -= x * align_; bloc_ -= idx_ / blocSize(); idx_ %= blocSize();}
      
      inline bool operator<(IteratorImpl b) const {return bloc_ != b.bloc_ ? (bloc_ < b.bloc_) : (idx_ < b.idx_);}
      inline bool operator<=(IteratorImpl b) const {return bloc_ != b.bloc_ ? (bloc_ <= b.bloc_) : (idx_ <= b.idx_);}
      inline bool operator>(IteratorImpl b) const {return bloc_ != b.bloc_ ? (bloc_ > b.bloc_) : (idx_ > b.idx_);}
      inline bool operator>=(IteratorImpl b) const {return bloc_ != b.bloc_ ? (bloc_ >= b.bloc_) : (idx_ >= b.idx_);}
      inline bool operator==(IteratorImpl b) const {return bloc_ == b.bloc_ && idx_ == b.idx_;}
      inline bool operator!=(IteratorImpl b) const {return !(*this == b);}
      
      inline BitField<T> operator*() const {return bloc_->test(idx_, width_);}
      inline BitField<T> operator[] (size_t i) const {return bloc_->test(idx_, width_);}
      
    private:
      IteratorImpl(std::vector<BitField<size_t>>::const_iterator bloc, size_t width, size_t align) : bloc_(bloc), width_(width), align_(align) {}
      IteratorImpl(std::vector<BitField<size_t>>::const_iterator bloc, size_t idx, size_t width, size_t align) : bloc_(bloc), idx_(idx), width_(width), align_(align) {}
      inline static size_t blocSize() {return sizeof(size_t) * 8;}
      
      std::vector<BitField<size_t>>::const_iterator bloc_;
      size_t idx_ = 0;
      size_t width_;
      size_t align_;
    };
    
  public:
    
    using const_iterator = IteratorImpl;
    
    inline Row() = default;
    inline Row(size_t cols, size_t width = 1) {resize(cols, width);}
    
    const_iterator begin() const {return const_iterator(elements_.begin(), width_, align_);}
    const_iterator end() const {return const_iterator(elements_.end(), width_, align_);}
    
    inline BitField<T> test(size_t j) const {j *= align_; return elements_[j/blocSize()].test(j%blocSize(), width_);}
    inline size_t first() const;
    inline size_t size(const std::array<size_t,2>& range) const;
    inline size_t width() const {return width_;}
    
    inline void set(size_t j, BitField<T> val = 1) {j *= align_; return elements_[j/blocSize()].set(j%blocSize(), val);}
    inline void clear(size_t j) {j *= align_; return elements_[j/blocSize()].clear(j%blocSize());}
    
    inline void swap(size_t a, size_t b);
    inline void move(size_t a, size_t b);
    
    inline Row operator() (const std::array<size_t,2>& colRange) const;
    
    inline void operator += (const Row& b);
    inline void operator -= (const Row& b);
    template <bool isConst, bool isTranslated, bool isScaled> inline void operator += (const typename SparseBitMatrix<T>::template RowRef<isConst,isTranslated,isScaled>& b);
    template <bool isConst, bool isTranslated, bool isScaled> inline void operator -= (const typename SparseBitMatrix<T>::template RowRef<isConst,isTranslated,isScaled>& b);
    inline void operator *= (int b);
    
    inline Row operator * (int b) const;
    
  private:
    template <typename Archive> void serialize(Archive & ar, const unsigned int version);
    
    inline void resize(size_t cols, size_t width);// {elements_.resize(((cols*align_)+blocSize()-1)/blocSize()); }
    static size_t blocSize() {return sizeof(size_t) * 8;}
    
    std::vector<BitField<size_t>> elements_;
    size_t width_;
    size_t align_;
  };
  
  using const_iterator = typename std::vector<Row>::const_iterator;
  using iterator = typename std::vector<Row>::iterator;
  
  inline BitMatrix() = default;
  inline BitMatrix(size_t rows, size_t cols, size_t width = 1) {resize(rows, cols, width);}
  inline BitMatrix(const SparseBitMatrix<T>& b);
  
  BitMatrix& operator = (const SparseBitMatrix<T>& b);
  
  inline void resize(size_t rows, size_t cols, size_t width = 1);
  
  inline size_t rows() const {return rows_.size();}
  inline size_t cols() const {return cols_;}
  inline size_t width() const;
  inline size_t size() const;
  
  inline BitMatrix operator() (const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange) const;
  
  inline iterator begin() {return rows_.begin();}
  inline iterator end() {return rows_.end();}
  inline const_iterator cbegin() const {return rows_.begin();}
  inline const_iterator cend() const {return rows_.end();}
  inline const_iterator begin() const {return rows_.begin();}
  inline const_iterator end() const {return rows_.end();}
  
  inline const Row& operator[] (size_t i) const {return rows_[i];}
  inline Row& operator[] (size_t i) {return rows_[i];}
  
  inline void colSizes(std::vector<size_t>& dst) const {colSizes({0, rows()}, {0, cols()}, dst);}
  inline void colSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& x) const;
  
  inline void rowSizes(std::vector<size_t>& dst) const {rowSizes({0, rows()}, {0, cols()}, dst);}
  inline void rowSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const;
  
  inline void moveCol(size_t a, size_t b);
  
  inline void swapCols(size_t a, size_t b) {swapCols(a, b, {0, rows()});}
  inline void swapCols(size_t a, size_t b, const std::array<size_t,2>& rowRange);
  
  inline void deleteRow(iterator a) {rows_.erase(a);}
  
private:
  template <typename Archive> void serialize(Archive & ar, const unsigned int version);
  
  size_t width_ = 1;
  size_t cols_ = 0;
  std::vector<Row> rows_;
};
  
}



template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
template <bool isConst2, bool isTranslated2, bool isScaled2>
void fec::SparseBitMatrix<T>::RowRef<isConst,isTranslated,isScaled>::operator = (SparseBitMatrix<T>::RowRef<isConst2,isTranslated2,isScaled2> b)
{
  rowIdx_.end = rowIdx_.begin + b.size();
  std::copy(b.begin(), b.end(), begin());
  for (auto elem = begin(); elem < end(); ++elem) {
    *elem.first -= b.translation();
    *elem.second *= b.scale();
  }
}

template <class T>
template <typename Archive>
void fec::SparseBitMatrix<T>::RowIdx::serialize(Archive & ar, const unsigned int version)
{
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(begin);
  ar & BOOST_SERIALIZATION_NVP(end);
}

template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
void fec::SparseBitMatrix<T>::RowRef<isConst,isTranslated,isScaled>::set(size_t i, BitField<T> val) const
{
  for (auto it = end()-1; it >= begin(); --it) {
    if (it->first > i) {
      it[1] = it[0];
    } else if (it->first < i) {
      it[1] = std::make_pair(i, val);
      ++rowIdx_.end;
      return;
    }
  }
  *begin() = std::make_pair(i, val);
  ++rowIdx_.end;
}

/**
 *  Access part of a row.
 *  This function returns a reference to a part of the bit row.
 *  \param  colRange  Range of the accessed bloc
 *  \return Reference to the bits in the bloc
 */
template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
fec::SparseBitMatrix<T>::RowRef<true,true,isScaled> fec::SparseBitMatrix<T>::RowRef<isConst,isTranslated,isScaled>::operator() (const std::array<size_t,2> colRange) const
{
  auto first = std::lower_bound(begin(), end(), colRange[0]);
  auto last = std::lower_bound(begin(), end(), colRange[1]);
  return RowRef<true,true,isScaled>(first, last, colRange[0])*scale();
}

/**
 *  Swaps two bits in a row.
 *  \param  a Index of the first bit
 *  \param  b Index of the second bit
 */
template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
void fec::SparseBitMatrix<T>::RowRef<isConst,isTranslated,isScaled>::swap(size_t a, size_t b)
{
  if (a > b) {
    std::swap(a,b);
  }
  auto aIt = std::lower_bound(begin(), end(), a, comp_);
  auto bIt = std::lower_bound(begin(), end(), b, comp_);
  if ((aIt != end() && aIt->first == a) && (bIt == end() || bIt->first != b)) {
    for (; aIt < end()-1 && b > aIt[1].first; aIt++) {
      aIt[0] = aIt[1];
    }
    aIt->first = b;
  }
  else if ((aIt == end() || aIt->first != a) && (bIt != end() && bIt->first == b)) {
    bIt--;
    for (; bIt >= begin() && a < bIt[0].first; bIt--) {
      bIt[1] = bIt[0];
    }
    bIt[1].first = a;
  }
}

/**
 *  Move one bits from a position the another one.
 *  All the bits in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
void fec::SparseBitMatrix<T>::RowRef<isConst,isTranslated,isScaled>::move(size_t a, size_t b)
{
  if (empty()) {
    return;
  }
  if (a > b) {
      auto aIt = std::lower_bound(begin(), end(), a);
      if (aIt != end() && *aIt == a) {
        aIt--;
        for (; aIt >= begin() && b <= *aIt; aIt--) {
          aIt[1] = aIt[0]+1;
        }
        aIt[1] = b;
      }
      else {
        aIt--;
        for (; aIt >= begin() && b <= *aIt; aIt--) {
          *aIt += 1;
        }
    }
  }
  else if (a < b) {
      auto aIt = std::lower_bound(begin(), end(), a);
      if (aIt != end() && *aIt == a) {
        for (; aIt < end()-1 && b >= aIt[1]; aIt++) {
          aIt[0] = aIt[1]-1;
        }
        *aIt = b;
      }
      else {
        for (; aIt < end() && b >= *aIt; aIt++) {
          *aIt -= 1;
        }
    }
  }
}

template <class T>
template <typename Archive>
void fec::SparseBitMatrix<T>::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(width_);
  ar & BOOST_SERIALIZATION_NVP(cols_);
  ar & BOOST_SERIALIZATION_NVP(elementIdx_);
  ar & BOOST_SERIALIZATION_NVP(rowIdx_);
}

/**
 *  Assignment operator.
 *  The method reorders data (if needed) to optimize access performance
 *  \param  b Bit matrix being copied
 */
template <class T>
fec::SparseBitMatrix<T> fec::SparseBitMatrix<T>::shrink() const
{
  std::vector<size_t> rs;
  rowSizes(rs);
  
  SparseBitMatrix<T> b;
  b.resize(rs, cols(), width());
  auto bRow = b.begin();
  for (auto row = begin(); row < end(); ++row, ++bRow) {
    *bRow = *row;
  }
  return b;
}

/**
 *  Assignment operator.
 *  The method reorders data (if needed) to optimize access performance
 *  \param  b Bit matrix being copied
 */
template <class T>
fec::SparseBitMatrix<T>& fec::SparseBitMatrix<T>::operator = (const fec::BitMatrix<T>& b)
{
  std::vector<size_t> rowSizes;
  b.rowSizes(rowSizes);
  
  resize(rowSizes, b.cols(), b.width());
  auto row = begin();
  for (auto bRow = b.begin(); bRow < b.end(); ++bRow, ++row) {
    for (size_t j = 0; j < b.cols(); ++j) {
      if (auto x = bRow->test(j)) {
        row->set(j, x);
      }
    }
  }
  return *this;
}

/**
 *  Access a submatrix specified by a row range and a column range.
 *  A copy of the data is performed.
 *  \param rowRange begin and end row index which the submatrix will contain.
 *  \param colRange begin and end column index which the submatrix will contain.
 *  \return submatrix
 */
template <class T>
fec::SparseBitMatrix<T> fec::SparseBitMatrix<T>::operator() (const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange) const
{
  std::vector<size_t> rs;
  rowSizes(rowRange, colRange, rs);
  
  SparseBitMatrix x(rs, colRange[1]-colRange[0]);
  auto xRow = x.begin();
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row, ++xRow) {
    *xRow = (*row)(colRange);
  }
  return x;
}

template <class T>
void fec::SparseBitMatrix<T>::colSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const
{
  dst.resize(colRange[1] - colRange[0], 0);
  std::fill(dst.begin(), dst.end(), 0);
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    auto elem = row->begin();
    while (elem->first < colRange[0]) ++elem;
    while (elem < row->end() && elem->first < colRange[1])
    {
      dst[elem->first]++;
      ++elem;
    }
  }
}

template <class T>
void fec::SparseBitMatrix<T>::rowSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const
{
  dst.resize(rowRange[1] - rowRange[0]);
  for (size_t i = 0; i < dst.size(); ++i) {
    dst[i] = (*this)[i+rowRange[0]].size(colRange);
  }
}

/**
 *  Move one column from a position the another one.
 *  All the columns in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
template <class T>
void fec::SparseBitMatrix<T>::moveCol(size_t a, size_t b)
{
  for (auto row = begin(); row < end(); ++row) {
    row->move(a,b);
  }
}

/**
 *  Swaps a part of two colums in the matrix.
 *  \param  a Index of the first column
 *  \param  b Index of the second column
 *  \param  rowRange  Begin and end row involved in the operation.
 */
template <class T>
void fec::SparseBitMatrix<T>::swapCols(size_t a, size_t b, const std::array<size_t,2>& rowRange)
{
  if (a==b) return;
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    row->swap(a,b);
  }
}

/**
 *  Return the transpose of the matrix.
 *  A copy of the data is performed.
 *  \return transposed matrix
 */
template <class T>
fec::SparseBitMatrix<T> fec::SparseBitMatrix<T>::transpose() const
{
  std::vector<size_t> cs;
  colSizes(cs);
  
  SparseBitMatrix x(cs, rows());
  auto xRow = x.begin();
  size_t i = 0;
  for (auto row = begin(); row < end(); ++row, ++i) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      xRow[elem->first].set(i);
    }
  }
  return x;
}

/**
 *  Resize the matrix to a specified shape.
 *  Data in the matrix may become invalid.
 *  \param  rows Number of row
 *  \param  cols Number of columns
 *  \param  rowSizes Number of non-zero element within each row
 */
template <class T>
void fec::SparseBitMatrix<T>::resize(size_t rows, size_t cols, size_t rowSizes, size_t width)
{
  width_ = width;
  cols_ = cols;
  elementIdx_.resize(rows * rowSizes);
  rowIdx_.resize(rows);
  for (auto rowIdx = rowIdx_.begin(); rowIdx < rowIdx_.end() - 1; ++rowIdx) {
    rowIdx[1].begin = rowIdx[0].end + rowSizes;
    rowIdx[1].end = rowIdx[1].begin;
  }
}

/**
 *  Resize the matrix to a specified shape.
 *  Data in the matrix may become invalid.
 *  \param  rowSizes Vector containing the number of non-zero element within each row
 *  \param  cols Number of columns
 */
template <class T>
void fec::SparseBitMatrix<T>::resize(const std::vector<size_t>& rowSizes, size_t cols, size_t width)
{
  width_ = width;
  cols_ = cols;
  if (rowSizes.size() == 0) return;
  rowIdx_.resize(rowSizes.size());
  for (size_t i = 0; i < rowSizes.size() - 1; ++i) {
    rowIdx_[i+1].begin = rowIdx_[i].end + rowSizes[i];
    rowIdx_[i+1].end = rowIdx_[i+1].begin;
  }
  elementIdx_.resize((rowIdx_.end()-1)->begin+*(rowSizes.end()-1), {});
}



template <class T>
template <typename Archive>
void fec::BitMatrix<T>::Row::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(elements_);
  ar & BOOST_SERIALIZATION_NVP(width_);
}

/**
 *  Access the index of the first non-zero element in a row.
 *  \return Index of the first non-zero element
 */
template <class T>
size_t fec::BitMatrix<T>::Row::first() const
{
  for (size_t i = 0; i < elements_.size(); ++i) {
    if (elements_[i] == 0) continue;
    for (size_t j = 0; j < blocSize(); ++j) {
      if (elements_[i].test(j)) return i*blocSize() + j;
    }
  }
  return -1;
}

/**
 *  Access the number of non-zero elements within a specified range of the row.
 *  \param  range begin and end index of considered elements
 *  \return Number of non-zero elements
 */
template <class T>
size_t fec::BitMatrix<T>::Row::size(const std::array<size_t,2>& range) const
{
  size_t x = 0;
  for (size_t i = range[0]; i < range[1]; ++i) {
    x += test(i);
  }
  return x;
}

/**
 *  Swaps two bits in a row.
 *  \param  a Index of the first bit
 *  \param  b Index of the second bit
 */
template <class T>
void fec::BitMatrix<T>::Row::swap(size_t a, size_t b)
{
  BitField<T> aVal = test(a, width());
  BitField<T> bVal = test(b, width());
  set(a, bVal, width());
  set(b, aVal, width());
}

/**
 *  Move one bits from a position the another one.
 *  All the bits in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
template <class T>
void fec::BitMatrix<T>::Row::move(size_t a, size_t b)
{
  if (a == b) {
    return;
  }
  
  a *= align_;
  b *= align_;
  size_t first = a;
  size_t last = b;
  if (a > b) {
    std::swap(first, last);
  }
  
  BitField<size_t> before = elements_[first/blocSize()] & ((size_t(1) << (first % blocSize())) - 1);
  BitField<size_t> after = elements_[(last+1)/blocSize()] & ~((size_t(1) << ((last+1) % blocSize())) - 1);
  
  size_t value = elements_[a/blocSize()].test(a%blocSize(), width());
  if (a < b) {
    if ((a+1) % blocSize() == 0) {
      elements_[a/blocSize()] = 0;
    }
    else {
      elements_[a/blocSize()] &= ~( (size_t(1) << ((a+1) % blocSize())) - 1 );
    }
    elements_[(b+1)/blocSize()] &= (size_t(1) << ((b+1) % blocSize())) - 1;
    
    size_t carry = 0;
    for (int64_t j = b/blocSize(); j >= int64_t(a/blocSize()); --j) {
      BitField<size_t> tmp = elements_[j].test(0, align_);
      elements_[j] >>= align_;
      elements_[j] |= carry << (blocSize()-align_);
      carry = tmp;
    }
  }
  else {
    elements_[a/blocSize()] &= ( (size_t(1) << (a % blocSize())) - 1 );
    elements_[b/blocSize()] &= ~( (size_t(1) << (b % blocSize())) - 1 );
    
    size_t carry = 0;
    for (size_t j = b/blocSize(); j <= a/blocSize(); ++j) {
      BitField<size_t> tmp = elements_[j].test(blocSize()-align_, align_);
      elements_[j] <<= align_;
      elements_[j] |= carry;
      carry = tmp;
    }
  }
  elements_[(last+1)/blocSize()] |= after;
  elements_[first/blocSize()] |= before;
  
  elements_[b/blocSize()] |= value << (b%blocSize());
}

/**
 *  Access part of a row.
 *  This function returns a reference to a part of the bit row.
 *  \param  colRange  Range of the accessed bloc
 *  \return Reference to the bits in the bloc
 */
template <class T>
typename fec::BitMatrix<T>::Row fec::BitMatrix<T>::Row::operator() (const std::array<size_t,2>& colRange) const
{
  Row x(colRange[1] - colRange[0], width());
  size_t offset = colRange[0]*align_ % blocSize();
  size_t i = 0; size_t j = colRange[0]*align_ / blocSize();
  x.elements_[i] = elements_[j] >> offset;
  ++i;
  for (; i < x.elements_.size() && j < elements_.size() - 1; ++i) {
    x.elements_[i] |= elements_[j] << (blocSize() - offset);
    ++j;
    x.elements_[i] = elements_[j] >> offset;
  }
  if (i < x.elements_.size()) {
    x.elements_[i] |= elements_[j] << (blocSize() - offset);
    x.elements_[i] &= ((size_t(1) << ((colRange[1] - colRange[0]) % blocSize())) - 1);//??
  }
  else {
    x.elements_[x.elements_.size()-1] &= ((size_t(1) << ((colRange[1] - colRange[0]) % blocSize())) - 1);
  }
  return x;
}

/**
 *  Addition assignement operator.
 *  This function compute implace the modulo-2 addition between 2 rows.
 *  \param  b Row being added the the member
 */
template <class T>
void fec::BitMatrix<T>::Row::operator += (const Row& b)
{
  BitField<size_t> mask = 0;
  for (size_t i = 0; i < blocSize(); i += 2*align_) {
    mask.set(i, align_);
  }
  for (size_t i = 0; i < elements_.size(); ++i) {
    elements_[i] += b.elements_[i];
    elements_[i] &= mask;
  }
}

/**
 *  Addition assignement operator.
 *  This function compute implace the modulo-2 addition between 2 rows.
 *  \param  b Row being added the the member
 */
template <class T>
template <bool isConst, bool isTranslated, bool isScaled>
void fec::BitMatrix<T>::Row::operator += (const typename SparseBitMatrix<T>::template RowRef<isConst,isTranslated,isScaled>& b)
{
  for (auto i = b.begin(); i != b.end(); ++i) {
    set(i->first, (test(i->first) + i->second) % width());
  }
}

template <class T>
fec::BitMatrix<T>::BitMatrix(const SparseBitMatrix<T>& b)
{
  resize(b.rows(), b.cols(), b.width());
  auto row = begin();
  for (auto bRow = b.begin(); bRow != b.end(); ++bRow, ++row) {
    for (auto j = bRow->begin(); j != bRow->end(); ++j) {
      row->set(j->first, j->second);
    }
  }
}

/**
 *  Access a submatrix specified by a row range and a column range.
 *  A copy of the data is performed.
 *  \param rowRange begin and end row index which the submatrix will contain.
 *  \param colRange begin and end column index which the submatrix will contain.
 *  \return submatrix
 */
template <class T>
fec::BitMatrix<T> fec::BitMatrix<T>::operator() (const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange) const
{
  BitMatrix x(rowRange[1]-rowRange[0], colRange[1]-colRange[0]);
  auto xRow = x.begin();
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row, ++xRow) {
    *xRow = (*row)(colRange);
  }
  return x;
}

/**
 *  Compute the column sizes in the matrix.
 *  We define the column size as the number of non-zero elements
 *  within the specified range of a column.
 *  A column size is given only for the column within the specified range.
 *  \param rowRange begin and end row index of considered elements
 *  \param colRange begin and end column index of considered elements
 *  \param[out] dst Vector containing the size of each column
 */
template <class T>
void fec::BitMatrix<T>::colSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& x) const
{
  x.resize(colRange[1] - colRange[0], 0);
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    for (size_t j = colRange[0]; j < colRange[1]; ++j) {
      if (row->test(j)) {
        x[j]++;
      }
    }
  }
}

/**
 *  Compute the row sizes in the matrix.
 *  We define the row size as the number of non-zero elements
 *  within the specified range of a row.
 *  A row size is given only for the row within the specified range.
 *  \param rowRange begin and end row index of considered elements
 *  \param colRange begin and end column index of considered elements
 *  \param[out] dst Vector containing the size of each row
 */
template <class T>
void fec::BitMatrix<T>::rowSizes(const std::array<size_t,2>& rowRange, const std::array<size_t,2>& colRange, std::vector<size_t>& dst) const
{
  dst.resize(rowRange[1] - rowRange[0]);
  for (size_t i = rowRange[0]; i < rowRange[1]; ++i) {
    dst[i] = rows_[i].size(colRange);
  }
}

template <class T>
void fec::BitMatrix<T>::swapCols(size_t a, size_t b, const std::array<size_t,2>& rowRange) {
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    row->swap(a, b);
  }
}

/**
 *  Move one column from a position the another one.
 *  All the columns in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
template <class T>
void fec::BitMatrix<T>::moveCol(size_t a, size_t b)
{
  for (auto row = begin(); row <  end(); ++row) {
    row->move(a, b);
  }
}

/**
 *  Resize the matrix to a specified shape.
 *  Data in the matrix may become invalid.
 *  \param  rows Number of row
 *  \param  cols Number of columns
 *  \param  rowSizes Number of non-zero element within each row
 */
template <class T>
void fec::BitMatrix<T>::resize(size_t rows, size_t cols, size_t width)
{
  width_ = width;
  cols_ = cols;
  rows_.resize(rows);
  for (auto row = begin(); row < end(); ++row) {
    row->resize(cols_);
  }
}

template <class T>
template <typename Archive>
void fec::BitMatrix<T>::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(width_);
  ar & BOOST_SERIALIZATION_NVP(cols_);
  ar & BOOST_SERIALIZATION_NVP(rows_);
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const fec::BitMatrix<T>& matrix)
{
  if (matrix.rows() == 0) {
    return os;
  }
  for (auto row = matrix.begin(); row < matrix.end(); ++row) {
    for (size_t j = 0; j < matrix.cols(); j++) {
      if (row->test(j)) {
        os << row->test(j);
      }
      else {
        os << 0;
      }
    }
    os << std::endl;
  }
  return os;
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const fec::SparseBitMatrix<T>& matrix)
{
  if (matrix.rows() == 0) {
    return os;
  }
  for (auto row = matrix.begin(); row < matrix.end(); ++row) {
    size_t j = 0;
    for (auto elem = row->begin(); j < matrix.cols() && elem < row->end(); ++j) {
      if (*elem == j) {
        os << row->test(j);
        ++elem;
      }
      else {
        os << 0;
      }
    }
    for (; j < matrix.cols(); ++j) {
      os << 0;
    }
    os << std::endl;
  }
  return os;
}

#endif
