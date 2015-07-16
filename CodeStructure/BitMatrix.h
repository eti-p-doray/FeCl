/*******************************************************************************
 *  \file BitMatrix.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-07-08
 *
 *  Declaration of the BitMatrix classes
 ******************************************************************************/

#ifndef BIT_MATRIX_H
#define BIT_MATRIX_H

#include <vector>
#include <iostream>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

#include "BitField.h"

class BitMatrix;
class SparseBitMatrix;

inline std::ostream& operator<<(std::ostream& os, const BitMatrix& mat);
inline std::ostream& operator<<(std::ostream& os, const SparseBitMatrix& mat);

/*******************************************************************************
 *  This class represents a sparse bit matrix.
 *  Only non-zero elements are stored.
 ******************************************************************************/
class SparseBitMatrix
{
  friend class boost::serialization::access;
  
  /*******************************************************************************
   *  This struct contains index to the bgin and end of a row.
   ******************************************************************************/
  struct RowIdx {
    size_t begin;
    size_t end;
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_NVP(begin);
      ar & BOOST_SERIALIZATION_NVP(end);
    }
  };
  
public:
  class Constrow;
  class RowRef;
  
  /*******************************************************************************
   *  This class is a const reference to a row with an offset.
   ******************************************************************************/
  class ConstOffsetRowRef
  {
    friend class SparseBitMatrix;
  public:
    inline std::vector<size_t>::const_iterator begin() const {return begin_;}
    inline std::vector<size_t>::const_iterator end() const {return end_;}
    inline size_t offset() const {return offset_;}
    inline size_t size() const {return end() - begin();}
    
  private:
    inline ConstOffsetRowRef() = default;
    inline ConstOffsetRowRef(std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end, size_t offset) {begin_ = begin; end_ = end; offset_ = offset;}
    
    std::vector<size_t>::const_iterator begin_;
    std::vector<size_t>::const_iterator end_;
    size_t offset_;
  };
  
  /*******************************************************************************
   *  This class is a const reference to a row.
   ******************************************************************************/
  class ConstRowRef
  {
    friend class SparseBitMatrix;
  public:
    ConstRowRef(const ConstRowRef&) = default;
    inline ConstRowRef(RowRef b) {begin_ = b.begin(); end_ = b.end();}
    
    inline std::vector<size_t>::const_iterator begin() const {return begin_;}
    inline std::vector<size_t>::const_iterator end() const {return end_;}
    
    inline bool test(size_t j) const {return binary_search(begin(), end(), j);}
    inline size_t first() const {return end() - begin() == 0 ? -1 : *begin();}
    inline size_t size() const {return end() - begin();}
    inline size_t size(const size_t range[2]) const {
      return std::lower_bound(begin(), end(), range[1]) - std::lower_bound(begin(), end(), range[0]);
    }
    inline size_t size(const std::initializer_list<size_t> colRange) const {return size(colRange.begin());}
    inline bool empty() const {return (end() - begin()) == 0;}
    
    inline ConstOffsetRowRef operator() (const std::initializer_list<size_t>& colRange) const {return (*this)(colRange.begin());}
    inline ConstOffsetRowRef operator() (const size_t colRange[2]) const;
    
  private:
    inline ConstRowRef() = default;
    inline ConstRowRef(std::vector<size_t>::const_iterator begin, std::vector<size_t>::const_iterator end) : begin_(begin), end_(end) {}
    
    std::vector<size_t>::const_iterator begin_;
    std::vector<size_t>::const_iterator end_;
  };
  
  /*******************************************************************************
   *  This class is a reference to a row.
   ******************************************************************************/
  class RowRef
  {
    friend class SparseBitMatrix;
    friend class ConstRow;
  public:
    RowRef(const RowRef&) = default;
    inline void operator = (ConstOffsetRowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      std::copy(b.begin(), b.end(), begin());
      for (auto elem = begin(); elem < end(); ++elem) {
        *elem -= b.offset_;
      }
    }
    inline void operator = (ConstRowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      std::copy(b.begin(), b.end(), begin());
    }
    inline void operator = (RowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      std::copy(b.begin(), b.end(), begin());
    }
    
    inline std::vector<size_t>::const_iterator begin() const {return begin_ + rowIdx_.begin;}
    inline std::vector<size_t>::const_iterator end() const {return begin_ + rowIdx_.end;}
    inline std::vector<size_t>::iterator begin() {return begin_ + rowIdx_.begin;}
    inline std::vector<size_t>::iterator end() {return begin_ + rowIdx_.end;}
    
    inline bool test(size_t j) const {return binary_search(begin(), end(), j);}
    inline size_t first() const {return end() - begin() == 0 ? -1 : *begin();}
    inline size_t size() const {return rowIdx_.end - rowIdx_.begin;}
    inline size_t size(const size_t range[2]) const {return std::lower_bound(begin(), end(), range[1]) - std::lower_bound(begin(), end(), range[0]);}
    inline size_t size(const std::initializer_list<size_t> colRange) const {return size(colRange.begin());}
    inline bool empty() const {return (end() - begin()) == 0;}
    
    inline void set(size_t j) {
      *end() = j;
      ++rowIdx_.end;
    }
    
    inline ConstOffsetRowRef operator() (const std::initializer_list<size_t>& colRange) const {return (*this)(colRange.begin());}
    inline ConstOffsetRowRef operator() (const size_t colRange[2]) const;
    
    inline void swap(size_t a, size_t b);
    inline void move(size_t a, size_t b);
    
    void swap(RowRef b) {
      std::swap(rowIdx_, b.rowIdx_);
    }
    
  private:
    inline RowRef() = default;
    inline RowRef(std::vector<size_t>::iterator begin, RowIdx& rowIdx) : begin_(begin), rowIdx_(rowIdx) {}
    
    std::vector<size_t>::iterator begin_;
    RowIdx& rowIdx_;
  };
  
  /*******************************************************************************
   *  This class emulates a ptr to a row.
   *  It contains a reference to the row.
   ******************************************************************************/
  class ConstRowPtr
  {
    friend class ConstIterator;
  public:
    ConstRowPtr(const ConstRowRef& row) : row_(row) {}
    inline const ConstRowRef* operator-> () const {return &row_;}
    
  private:
    ConstRowRef row_;
  };
  class RowPtr
  {
    friend class Iterator;
  public:
    RowPtr(const RowRef& row) : row_(row) {}
    inline RowRef* operator-> () {return &row_;}
    
  private:
    RowRef row_;
  };
  
  /*******************************************************************************
   *  This is a random access input iterator of a sparse matrix.
   *  It iterates over the matrix rows.
   ******************************************************************************/
  class ConstIterator
  {
    friend class SparseBitMatrix;
  public:
    inline void operator++() {++rowIdx_;}
    inline void operator--() {--rowIdx_;}
    
    inline void operator+=(size_t x) {rowIdx_ += x;}
    inline void operator-=(size_t x) {rowIdx_ -= x;}
    
    inline bool operator<(ConstIterator b) const {return rowIdx_ < b.rowIdx_;}
    inline bool operator<=(ConstIterator b) const {return rowIdx_ <= b.rowIdx_;}
    inline bool operator>(ConstIterator b) const {return rowIdx_ > b.rowIdx_;}
    inline bool operator>=(ConstIterator b) const {return rowIdx_ >= b.rowIdx_;}
    inline bool operator==(ConstIterator b) const {return rowIdx_ == b.rowIdx_;}
    inline bool operator!=(ConstIterator b) const {return rowIdx_ != b.rowIdx_;}
    
    template <typename T> friend ConstIterator operator+(const ConstIterator& a, T b) {return ConstIterator(a.begin_, a.rowIdx_+b);}
    template <typename T> friend ConstIterator operator+(T a, const ConstIterator& b) {return ConstIterator(b.begin_, b.rowIdx_+a);}
    
    template <typename T> friend ConstIterator operator-(const ConstIterator& a, T b)  {return ConstIterator(a.begin_, a.rowIdx_-b);}
    inline friend size_t operator-(const ConstIterator& a, const ConstIterator& b) {return a.rowIdx_ - b.rowIdx_;}
    
    inline ConstRowRef operator*() const {return ConstRowRef(begin_ + rowIdx_->begin, begin_ + rowIdx_->end);}
    inline ConstRowPtr operator-> () const {return ConstRowPtr(*(*this));}
    inline ConstRowRef operator[] (size_t i) const {return ConstRowRef(begin_ + rowIdx_[i].begin, begin_ + rowIdx_[i].end);}
    
  private:
    inline ConstIterator(std::vector<size_t>::const_iterator begin, std::vector<RowIdx>::const_iterator rowIdx) :begin_(begin), rowIdx_(rowIdx) {}
    
    std::vector<size_t>::const_iterator begin_;
    std::vector<RowIdx>::const_iterator rowIdx_;
  };
  /*******************************************************************************
   *  This is a random access iterator of a sparse matrix.
   *  It iterates over the matrix rows.
   ******************************************************************************/
  class Iterator
  {
    friend class SparseBitMatrix;
  public:
    inline void operator++() {++rowIdx_;}
    inline void operator--() {--rowIdx_;}
    
    inline void operator+=(size_t x) {rowIdx_ += x;}
    inline void operator-=(size_t x) {rowIdx_ -= x;}
    
    inline bool operator<(Iterator b) const {return rowIdx_ < b.rowIdx_;}
    inline bool operator<=(Iterator b) const {return rowIdx_ <= b.rowIdx_;}
    inline bool operator>(Iterator b) const {return rowIdx_ > b.rowIdx_;}
    inline bool operator>=(Iterator b) const {return rowIdx_ >= b.rowIdx_;}
    inline bool operator==(Iterator b) const {return rowIdx_ == b.rowIdx_;}
    inline bool operator!=(Iterator b) const {return rowIdx_ != b.rowIdx_;}
    
    template <typename T> friend Iterator operator+(const Iterator& a, T b) {return Iterator(a.begin_, a.rowIdx_+b);}
    template <typename T> friend Iterator operator+(T a, const Iterator& b) {return Iterator(b.begin_, b.rowIdx_+a);}
    
    template <typename T> friend Iterator operator-(const Iterator& a, T b)  {return Iterator(a.begin_, a.rowIdx_-b);}
    inline friend size_t operator-(const Iterator& a, const Iterator& b) {return a.rowIdx_ - b.rowIdx_;}
    
    inline RowRef operator*() {return RowRef(begin_, *rowIdx_);}
    inline RowPtr operator-> () {return RowPtr(*(*this));}
    inline RowRef operator[] (size_t i) {return RowRef(begin_, rowIdx_[i]);}
    
    inline ConstRowRef operator*() const {return ConstRowRef(begin_ + rowIdx_->begin, begin_ + rowIdx_->end);}
    inline ConstRowPtr operator-> () const {return ConstRowPtr(*(*this));}
    inline ConstRowRef operator[] (size_t i) const {return ConstRowRef(begin_ + rowIdx_[i].begin, begin_ + rowIdx_[i].end);}
    
  private:
    inline Iterator(std::vector<size_t>::iterator begin, std::vector<RowIdx>::iterator rowIdx) : begin_(begin), rowIdx_(rowIdx) {}
    
    std::vector<size_t>::iterator begin_;
    std::vector<RowIdx>::iterator rowIdx_;
  };
  
  SparseBitMatrix() = default;
  SparseBitMatrix(const SparseBitMatrix& b) {*this = b;}
  SparseBitMatrix(const BitMatrix& b) {*this = b;}
  SparseBitMatrix(SparseBitMatrix&& b) {cols_ = b.cols_; std::swap(elementIdx_, b.elementIdx_); std::swap(rowIdx_, b.rowIdx_);}
  inline SparseBitMatrix(size_t rows, size_t cols, size_t rowSizes) {resize(rows, cols, rowSizes);}
  inline SparseBitMatrix(const std::vector<size_t>& rowSizes, size_t cols) {resize(rowSizes, cols);}
  
  inline SparseBitMatrix& operator = (const SparseBitMatrix& b);
  inline SparseBitMatrix& operator = (const BitMatrix& b);
  inline SparseBitMatrix& operator = (SparseBitMatrix&& b) {cols_ = b.cols_; std::swap(elementIdx_, b.elementIdx_); std::swap(rowIdx_, b.rowIdx_); return *this;}
  
  inline void resize(size_t rows, size_t cols, size_t rowSizes);
  inline void resize(const std::vector<size_t>& rowSizes, size_t cols);
  
  inline size_t rows() const {return rowIdx_.size();}
  inline size_t cols() const {return cols_;}
  inline size_t size() const {return elementIdx_.size();}
  
  inline SparseBitMatrix operator() (const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange) const {return (*this)(rowRange.begin(), colRange.begin());}
  inline SparseBitMatrix operator() (const size_t rowRange[2], const size_t colRange[2]) const;
  
  inline ConstIterator begin() const {return ConstIterator(elementIdx_.begin(), rowIdx_.begin());}
  inline ConstIterator end() const {return ConstIterator(elementIdx_.begin(), rowIdx_.end());}
  inline ConstIterator cbegin() const {return ConstIterator(elementIdx_.begin(), rowIdx_.begin());}
  inline ConstIterator cend() const {return ConstIterator(elementIdx_.begin(), rowIdx_.end());}
  inline Iterator begin() {return Iterator(elementIdx_.begin(), rowIdx_.begin());}
  inline Iterator end() {return Iterator(elementIdx_.begin(), rowIdx_.end());}
  
  inline ConstRowRef operator[] (size_t i) const {return ConstRowRef(elementIdx_.begin() + rowIdx_[i].begin, elementIdx_.begin() + rowIdx_[i].end);}
  inline RowRef operator[] (size_t i) {return RowRef(elementIdx_.begin(), rowIdx_[i]);}
  
  inline void colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const;
  inline void colSizes(std::vector<size_t>& dst) const {colSizes({0, rows()}, {0, cols()}, dst);}
  inline void colSizes(const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange, std::vector<size_t>& x) const {colSizes(rowRange.begin(), colRange.begin(), x);}
  
  inline void rowSizes(std::vector<size_t>& x) const {rowSizes({0, rows()}, {0, cols()}, x);}
  inline void rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& x) const;
  inline void rowSizes(const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange, std::vector<size_t>& x) const {rowSizes(rowRange.begin(), colRange.begin(), x);}
  
  inline void moveCol(size_t a, size_t b);
  
  inline void swapCols(size_t a, size_t b) {swapCols(a, b, {0, rows()});}
  inline void swapCols(size_t a, size_t b, const size_t rowRange[2]);
  inline void swapCols(size_t a, size_t b, const std::initializer_list<size_t>& rowRange) {swapCols(a, b, rowRange.begin());}
  
  inline SparseBitMatrix transpose() const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(cols_);
    ar & BOOST_SERIALIZATION_NVP(elementIdx_);
    ar & BOOST_SERIALIZATION_NVP(rowIdx_);
  }
  
  size_t cols_;
  std::vector<size_t> elementIdx_;
  std::vector<RowIdx> rowIdx_;
};

namespace std {
  inline void swap(SparseBitMatrix::RowRef a, SparseBitMatrix::RowRef b) {
    a.swap(b);
  }
}


/*******************************************************************************
 *  This class represents a full bit matrix.
 *  Every bit is stored in a compact BitField.
 ******************************************************************************/
class BitMatrix
{
  friend class boost::serialization::access;
public:
  /*******************************************************************************
   *  This class is a full bit row.
   ******************************************************************************/
  class Row
  {
    friend class boost::serialization::access;
    friend class BitMatrix;
  public:
    class ConstIterator {
      friend class Row;
    public:
      inline void operator++() {++idx_; if (idx_ == blocSize()) {++bloc_; idx_ = 0;}}
      inline void operator--() {--idx_; if (idx_ == 0) {--bloc_; idx_ = blocSize()-1;}}
      
      inline void operator+=(size_t x) {idx_ += x; bloc_ += idx_ / blocSize(); idx_ %= blocSize();}
      inline void operator-=(size_t x) {idx_ -= x; bloc_ += idx_ / blocSize(); idx_ %= blocSize();}
      
      inline bool operator<(ConstIterator b) const {return bloc_ != b.bloc_ ? (bloc_ < b.bloc_) : (idx_ < b.idx_);}
      inline bool operator<=(ConstIterator b) const {return bloc_ != b.bloc_ ? (bloc_ <= b.bloc_) : (idx_ <= b.idx_);}
      inline bool operator>(ConstIterator b) const {return bloc_ != b.bloc_ ? (bloc_ > b.bloc_) : (idx_ > b.idx_);}
      inline bool operator>=(ConstIterator b) const {return bloc_ != b.bloc_ ? (bloc_ >= b.bloc_) : (idx_ >= b.idx_);}
      inline bool operator==(ConstIterator b) const {return bloc_ == b.bloc_ && idx_ == b.idx_;}
      inline bool operator!=(ConstIterator b) const {return !(*this == b);}
      
      inline bool operator*() const {return bloc_->test(idx_);}
      
    private:
      ConstIterator(std::vector<BitField<size_t>>::const_iterator bloc) : bloc_(bloc) {}
      ConstIterator(std::vector<BitField<size_t>>::const_iterator bloc, size_t idx) : bloc_(bloc), idx_(idx) {}
      static size_t blocSize() {return sizeof(size_t) * 8;}
      
      std::vector<BitField<size_t>>::const_iterator bloc_;
      size_t idx_ = 0;
    };
    
    inline Row() = default;
    inline Row(size_t cols) {resize(cols);}
    
    ConstIterator begin() const {return ConstIterator(elements_.begin());}
    ConstIterator end() const {return ConstIterator(elements_.end());}
    
    inline bool test(size_t j) const {return elements_[j/blocSize()].test(j%blocSize());}
    inline size_t first() const;
    inline size_t size(const size_t range[2]) const;
    
    inline void set(size_t j) {return elements_[j/blocSize()].set(j%blocSize());}
    inline void set(size_t j, bool val) {return elements_[j/blocSize()].set(j%blocSize(), val);}
    inline void toggle(size_t j) {return elements_[j/blocSize()].toggle(j%blocSize());}
    inline void clear(size_t j) {return elements_[j/blocSize()].clear(j%blocSize());}
    
    inline void swap(size_t a, size_t b);
    inline void move(size_t a, size_t b);
    
    inline Row operator() (const std::initializer_list<size_t>& colRange) const {return (*this)(colRange.begin());}
    inline Row operator() (const size_t colRange[2]) const;
    
    inline void operator += (const Row& b);
    inline void operator += (SparseBitMatrix::ConstRowRef b);
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_NVP(elements_);
    }
    
    inline void resize(size_t cols) {elements_.resize((cols+blocSize()-1)/blocSize());}
    static size_t blocSize() {return sizeof(size_t) * 8;}
    
    std::vector<BitField<size_t>> elements_;
  };
  
  inline BitMatrix() = default;
  inline BitMatrix(size_t rows, size_t cols) {resize(rows, cols);}
  inline BitMatrix(const SparseBitMatrix& b);
  
  inline size_t rows() const {return rows_.size();}
  inline size_t cols() const {return cols_;}
  inline size_t size() const;
  
  inline BitMatrix operator() (const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange) {return (*this)(rowRange.begin(), colRange.begin());}
  inline BitMatrix operator() (const size_t rowRange[2], const size_t colRange[2]) const;
  
  inline std::vector<Row>::iterator begin() {return rows_.begin();}
  inline std::vector<Row>::iterator end() {return rows_.end();}
  inline std::vector<Row>::const_iterator begin() const {return rows_.begin();}
  inline std::vector<Row>::const_iterator end() const {return rows_.end();}
  
  inline const Row& operator[] (size_t i) const {return rows_[i];}
  inline Row& operator[] (size_t i) {return rows_[i];}
  
  inline void colSizes(std::vector<size_t>& dst) const {colSizes({0, rows()}, {0, cols()}, dst);}
  inline void colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& x) const;
  inline void colSizes(const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange, std::vector<size_t>& x) const {colSizes(rowRange.begin(), colRange.begin(), x);}
  
  inline void rowSizes(std::vector<size_t>& dst) const {rowSizes({0, rows()}, {0, cols()}, dst);}
  inline void rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const;
  inline void rowSizes(const std::initializer_list<size_t>& rowRange, const std::initializer_list<size_t>& colRange, std::vector<size_t>& x) const  {rowSizes(rowRange.begin(), colRange.begin(), x);}
  
  inline void moveCol(size_t a, size_t b);
  
  inline void swapCols(size_t a, size_t b) {swapCols(a, b, {0, rows()});}
  inline void swapCols(size_t a, size_t b, const size_t rowRange[2]) {
    for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
      row->swap(a, b);
    }
  }
  inline void swapCols(size_t a, size_t b, const std::initializer_list<size_t>& rowRange) {swapCols(a, b, rowRange.begin());}
  
  inline void deleteRow(std::vector<Row>::iterator a) {rows_.erase(a);}
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(cols_);
    ar & BOOST_SERIALIZATION_NVP(rows_);
  }
  
  inline void resize(size_t rows, size_t cols);
  
  size_t cols_;
  std::vector<Row> rows_;
};



SparseBitMatrix::ConstOffsetRowRef SparseBitMatrix::RowRef::operator() (const size_t colRange[2]) const
{
  auto first = std::lower_bound(begin(), end(), colRange[0]);
  auto last = std::lower_bound(begin(), end(), colRange[1]);
  return ConstOffsetRowRef(first, last, colRange[0]);
}

SparseBitMatrix::ConstOffsetRowRef SparseBitMatrix::ConstRowRef::operator() (const size_t colRange[2]) const
{
  auto first = std::lower_bound(begin(), end(), colRange[0]);
  auto last = std::lower_bound(begin(), end(), colRange[1]);
  return ConstOffsetRowRef(first, last, colRange[0]);
}

void SparseBitMatrix::RowRef::swap(size_t a, size_t b)
{
  if (a > b) {
    std::swap(a,b);
  }
  auto aIt = std::lower_bound(begin(), end(), a);
  auto bIt = std::lower_bound(begin(), end(), b);
  if ((aIt != end() && *aIt == a) && (bIt == end() || *bIt != b)) {
    for (; aIt < end()-1 && b > aIt[1]; aIt++) {
      aIt[0] = aIt[1];
    }
    *aIt = b;
  }
  else if ((aIt == end() || *aIt != a) && (bIt != end() && *bIt == b)) {
    bIt--;
    for (; bIt >= begin() && a < bIt[0]; bIt--) {
      bIt[1] = bIt[0];
    }
    bIt[1] = a;
  }
}

void SparseBitMatrix::RowRef::move(size_t a, size_t b)
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

SparseBitMatrix& SparseBitMatrix::operator = (const SparseBitMatrix& b)
{
  std::vector<size_t> rowSizes;
  b.rowSizes(rowSizes);
  
  resize(rowSizes, b.cols());
  auto row = begin();
  for (auto bRow = b.begin(); bRow < b.end(); ++bRow, ++row) {
    *row = *bRow;
  }
  return *this;
}

SparseBitMatrix& SparseBitMatrix::operator = (const BitMatrix& b)
{
  std::vector<size_t> rowSizes;
  b.rowSizes(rowSizes);
  
  resize(rowSizes, b.cols());
  auto row = begin();
  for (auto bRow = b.begin(); bRow < b.end(); ++bRow, ++row) {
    for (size_t j = 0; j < b.cols(); ++j) {
      if (bRow->test(j)) {
        row->set(j);
      }
    }
  }
  return *this;
}

SparseBitMatrix SparseBitMatrix::operator() (const size_t rowRange[2], const size_t colRange[2]) const
{
  std::vector<size_t> rowSizes;
  this->rowSizes(rowRange, colRange, rowSizes);
  
  SparseBitMatrix x(rowSizes, colRange[1]-colRange[0]);
  auto xRow = x.begin();
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row, ++xRow) {
    *xRow = (*row)(colRange);
  }
  return x;
}

void SparseBitMatrix::colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
{
  dst.resize(colRange[1] - colRange[0]);
  std::fill(dst.begin(), dst.end(), 0);
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    auto elem = row->begin();
    while (*elem < colRange[0]) ++elem;
    while (elem < row->end() && *elem < colRange[1])
    {
      dst[*elem]++;
      ++elem;
    }
  }
}

void SparseBitMatrix::rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
{
  dst.resize(rowRange[1] - rowRange[0]);
  for (size_t i = 0; i < dst.size(); ++i) {
    dst[i] = (*this)[i+rowRange[0]].size(colRange);
  }
}

void SparseBitMatrix::moveCol(size_t a, size_t b)
{
  for (auto row = begin(); row < end(); ++row) {
    row->move(a,b);
  }
}

void SparseBitMatrix::swapCols(size_t a, size_t b, const size_t rowRange[2])
{
  if (a==b) return;
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row) {
    row->swap(a,b);
  }
}

SparseBitMatrix SparseBitMatrix::transpose() const
{
  std::vector<size_t> colSizes;
  this->colSizes(colSizes);
  
  SparseBitMatrix x(colSizes, rows());
  auto xRow = x.begin();
  size_t i = 0;
  for (auto row = begin(); row < end(); ++row, ++i) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      xRow[*elem].set(i);
    }
  }
  return x;
}


void SparseBitMatrix::resize(size_t rows, size_t cols, size_t rowSizes)
{
  cols_ = cols;
  elementIdx_.resize(rows * rowSizes);
  rowIdx_.resize(rows);
  for (auto rowIdx = rowIdx_.begin(); rowIdx < rowIdx_.end() - 1; ++rowIdx) {
    rowIdx[1].begin = rowIdx[0].end + rowSizes;
    rowIdx[1].end = rowIdx[1].begin;
  }
}

void SparseBitMatrix::resize(const std::vector<size_t>& rowSizes, size_t cols)
{
  cols_ = cols;
  if (rowSizes.size() == 0) return;
  rowIdx_.resize(rowSizes.size());
  for (size_t i = 0; i < rowSizes.size() - 1; ++i) {
    rowIdx_[i+1].begin = rowIdx_[i].end + rowSizes[i];
    rowIdx_[i+1].end = rowIdx_[i+1].begin;
  }
  elementIdx_.resize((rowIdx_.end()-1)->begin+*(rowSizes.end()-1), 0);
}




size_t BitMatrix::Row::first() const
{
  for (size_t i = 0; i < elements_.size(); ++i) {
    if (elements_[i] == 0) continue;
    for (size_t j = 0; j < blocSize(); ++j) {
      if (elements_[i].test(j)) return i*blocSize() + j;
    }
  }
  return -1;
}

size_t BitMatrix::Row::size(const size_t range[2]) const
{
  size_t x = 0;
  for (size_t i = range[0]; i < range[1]; ++i) {
    x += test(i);
  }
  return x;
}

void BitMatrix::Row::swap(size_t a, size_t b)
{
  bool aVal = test(a);
  bool bVal = test(b);
  bool tmp = aVal ^ bVal;
  set(a, aVal ^ tmp);
  set(b, bVal ^ tmp);
}

void BitMatrix::Row::move(size_t a, size_t b)
{
  if (a == b) {
    return;
  }
  
  size_t first = a;
  size_t last = b;
  if (a > b) {
    std::swap(first, last);
  }
  
  BitField<size_t> before = elements_[first/blocSize()] & ((size_t(1) << (first % blocSize())) - 1);
  BitField<size_t> after = elements_[(last+1)/blocSize()] & ~((size_t(1) << ((last+1) % blocSize())) - 1);
  
  size_t value = elements_[a/blocSize()].test(a%blocSize());
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
      bool tmp = elements_[j].test(0);
      elements_[j] >>= 1;
      elements_[j] |= carry << (blocSize()-1);
      carry = tmp;
    }
  }
  else {
    elements_[a/blocSize()] &= ( (size_t(1) << (a % blocSize())) - 1 );
    elements_[b/blocSize()] &= ~( (size_t(1) << (b % blocSize())) - 1 );
    
    size_t carry = 0;
    for (size_t j = b/blocSize(); j <= a/blocSize(); ++j) {
      bool tmp = elements_[j].test(blocSize()-1);
      elements_[j] <<= 1;
      elements_[j] |= carry;
      carry = tmp;
    }
  }
  elements_[(last+1)/blocSize()] |= after;
  elements_[first/blocSize()] |= before;
  
  elements_[b/blocSize()] |= value << (b%blocSize());
}

BitMatrix::Row BitMatrix::Row::operator() (const size_t colRange[2]) const
{
  Row x(colRange[1] - colRange[0]);
  size_t offset = colRange[0] % blocSize();
  size_t i = 0; size_t j = colRange[0] / blocSize();
  x.elements_[i] = elements_[j] >> offset;
  ++i;
  for (; i < x.elements_.size() && j < elements_.size() - 1; ++i) {
    x.elements_[i] |= elements_[j] << (blocSize() - offset);
    ++j;
    x.elements_[i] = elements_[j] >> offset;
  }
  if (i < x.elements_.size()) {
    x.elements_[i] |= elements_[j] << (blocSize() - offset);
    x.elements_[i] &= ((size_t(1) << ((colRange[1] - colRange[0]) % blocSize())) - 1);
  }
  else {
    x.elements_[x.elements_.size()-1] &= ((size_t(1) << ((colRange[1] - colRange[0]) % blocSize())) - 1);
  }
  return x;
}

void BitMatrix::Row::operator += (const Row& b)
{
  for (size_t i = 0; i < elements_.size(); ++i) {
    elements_[i] ^= b.elements_[i];
  }
}

void BitMatrix::Row::operator += (SparseBitMatrix::ConstRowRef b)
{
  for (auto bRow = b.begin(); bRow < b.end(); ++bRow) {
    elements_[*bRow/blocSize()].toggle(*bRow%blocSize());
  }
}

BitMatrix::BitMatrix(const SparseBitMatrix& b)
{
  resize(b.rows(), b.cols());
  auto row = begin();
  for (auto bRow = b.begin(); bRow != b.end(); ++bRow, ++row) {
    auto bElem = bRow->begin();
    for (size_t j = 0; j < b.cols() && bElem < bRow->end(); ++j) {
      if (*bElem == j) {
        row->set(j);
        ++bElem;
      }
    }
  }
}

BitMatrix BitMatrix::operator() (const size_t rowRange[2], const size_t colRange[2]) const
{
  BitMatrix x(rowRange[1]-rowRange[0], colRange[1]-colRange[0]);
  auto xRow = x.begin();
  for (auto row = begin()+rowRange[0]; row < begin()+rowRange[1]; ++row, ++xRow) {
    *xRow = (*row)(colRange);
  }
  return x;
}

void BitMatrix::colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& x) const
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

void BitMatrix::rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
{
  dst.resize(rowRange[1] - rowRange[0]);
  for (size_t i = rowRange[0]; i < rowRange[1]; ++i) {
    dst[i] = rows_[i].size(colRange);
  }
}

void BitMatrix::moveCol(size_t a, size_t b)
{
  for (auto row = begin(); row <  end(); ++row) {
    row->move(a, b);
  }
}

void BitMatrix::resize(size_t rows, size_t cols)
{
  cols_ = cols;
  rows_.resize(rows);
  for (auto row = begin(); row < end(); ++row) {
    row->resize(cols_);
  }
}

std::ostream& operator<<(std::ostream& os, const BitMatrix& matrix)
{
  if (matrix.rows() == 0) {
    return os;
  }
  for (auto row = matrix.begin(); row < matrix.end(); ++row) {
    for (size_t j = 0; j < matrix.cols(); j++) {
      if (row->test(j)) {
        os << 1;
      }
      else {
        os << 0;
      }
    }
    os << std::endl;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const SparseBitMatrix& matrix)
{
  if (matrix.rows() == 0) {
    return os;
  }
  for (auto row = matrix.begin(); row < matrix.end(); ++row) {
    size_t j = 0;
    for (auto elem = row->begin(); j < matrix.cols() && elem < row->end(); ++j) {
      if (*elem == j) {
        os << 1;
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
