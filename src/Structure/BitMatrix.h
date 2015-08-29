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
 
 Declaration of the BitMatrix classes
 ******************************************************************************/

#ifndef BIT_MATRIX_H
#define BIT_MATRIX_H

#include <iostream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

#include "BitField.h"

namespace fec {
  
class BitMatrix;
class SparseBitMatrix;

/**
 *  This class represents a sparse bit matrix.
 *  Only non-zero elements are stored.
 *  The class defines basic methods to manipulate the matrix such as row/column swapping.
 */
class SparseBitMatrix
{
  friend class ::boost::serialization::access;
  
  /**
   *  This struct contains index to the begining and end of a row.
   */
  struct RowIdx {
    size_t begin;
    size_t end;
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & ::BOOST_SERIALIZATION_NVP(begin);
      ar & ::BOOST_SERIALIZATION_NVP(end);
    }
  };
  
public:
  class Constrow;
  class RowRef;
  
  /**
   *  This is a const reference to a SparseBitMatrix row with an offset.
   */
  class ConstOffsetRowRef
  {
    friend class SparseBitMatrix;
  public:
    /**
     * Access a random access input iterator pointing to the index of the first non-zero element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access input iterator
     */
    inline ::std::vector<size_t>::const_iterator begin() const {return begin_;}
    /**
     * Access a random access input iterator referring to the past-the-end element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access input iterator
     */
    inline ::std::vector<size_t>::const_iterator end() const {return end_;}
    /**
     * Access the offset value of the object.
     *  \return The offset value
     */
    inline size_t offset() const {return offset_;}
    /**
     * Access the size of the row.
     *  We define the size as the number of non-zero elements.
     *  \return Size of the row
     */
    inline size_t size() const {return end() - begin();}
    
  private:
    inline ConstOffsetRowRef() = default;
    inline ConstOffsetRowRef(::std::vector<size_t>::const_iterator begin, ::std::vector<size_t>::const_iterator end, size_t offset) {begin_ = begin; end_ = end; offset_ = offset;}
    
    ::std::vector<size_t>::const_iterator begin_;
    ::std::vector<size_t>::const_iterator end_;
    size_t offset_;
  };
  
  /**
   *  This is a const reference to a SparseBitMatrix row.
   */
  class ConstRowRef
  {
    friend class SparseBitMatrix;
  public:
    ConstRowRef(const ConstRowRef&) = default;
    /**
     *  Converts a SparseBitMatrix::RowRef to a SparseBitMatrix::ConstRowRef
     */
    inline ConstRowRef(RowRef b) {begin_ = b.begin(); end_ = b.end();}
    
    /**
     * Access a random access input iterator pointing to the index of the first non-zero element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access input iterator
     */
    inline ::std::vector<size_t>::const_iterator begin() const {return begin_;}
    /**
     * Access a random access input iterator referring to the past-the-end element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access input iterator
     */
    inline ::std::vector<size_t>::const_iterator end() const {return end_;}
    
    /**
     * Access the value of one element in the row.
     *  \param  j Index of the accessed value
     *  \return Value of the element
     */
    inline bool test(size_t j) const {return std::binary_search(begin(), end(), j);}
    /**
     *  Access the index of the first non-zero element in a row.
     *  \return Index of the first non-zero element
     */
    inline size_t first() const {return end() - begin() == 0 ? -1 : *begin();}
    /**
     *  Access the number of non-zero elements in a row.
     *  \return Number of non-zero elements
     */
    inline size_t size() const {return end() - begin();}
    /**
     *  Access the number of non-zero elements within a specified range of the row.
     *  \param  range begin and end index of considered elements
     *  \return Number of non-zero elements
     */
    inline size_t size(const size_t range[2]) const {
      return ::std::lower_bound(begin(), end(), range[1]) - ::std::lower_bound(begin(), end(), range[0]);
    }
    /**
     *  Access the number of non-zero elements within a specified range of the row.
     *  \param  range begin and end index of considered elements
     *  \return Number of non-zero elements
     */
    inline size_t size(const ::std::initializer_list<size_t> colRange) const {return size(colRange.begin());}
    /**
     *  \return True if the row contains no non-zero elements
     */
    inline bool empty() const {return (end() - begin()) == 0;}
    
    /**
     *  Access part of a row specified by an index range.
     *  This method returns a reference to a part of the bit row.
     *  \param  colRange  Range of the accessed bloc
     *  \return Reference to the bits in the bloc
     */
    inline ConstOffsetRowRef operator() (const ::std::initializer_list<size_t>& colRange) const {return (*this)(colRange.begin());}
    inline ConstOffsetRowRef operator() (const size_t colRange[2]) const;
    
  private:
    inline ConstRowRef() = default;
    inline ConstRowRef(::std::vector<size_t>::const_iterator begin, ::std::vector<size_t>::const_iterator end) : begin_(begin), end_(end) {}
    
    ::std::vector<size_t>::const_iterator begin_;
    ::std::vector<size_t>::const_iterator end_;
  };
  
  /**
   *  This ss a reference to a SparseBitMatrix row.
   */
  class RowRef
  {
    friend class SparseBitMatrix;
    friend class ConstRow;
  public:
    RowRef(const RowRef&) = default;
    /**
     *  Assign the data of a SparseBitMatrix::ConstOffsetRowRef to the data
     *  referred by the object.
     *  This will yield a copy of the underlying data.
     *  The object must be a valid reference to sufficiently allocated space.
     */
    inline void operator = (ConstOffsetRowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      ::std::copy(b.begin(), b.end(), begin());
      for (auto elem = begin(); elem < end(); ++elem) {
        *elem -= b.offset_;
      }
    }
    /**
     *  Assign the data of a SparseBitMatrix::ConstRowRef to the data
     *  referred by the object.
     *  This will yield a copy of the underlying data.
     *  The object must be a valid reference to sufficiently allocated space.
     */
    inline void operator = (ConstRowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      ::std::copy(b.begin(), b.end(), begin());
    }
    /**
     *  Assign the data of a SparseBitMatrix::RowRef to the data
     *  referred by the object.
     *  This will yield a copy of the underlying data.
     *  The object must be a valid reference to sufficiently allocated space.
     */
    inline void operator = (RowRef b) {
      rowIdx_.end = rowIdx_.begin + b.size();
      ::std::copy(b.begin(), b.end(), begin());
    }
    
    /**
     * Access a random access iterator pointing to the index of the first non-zero element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access iterator
     */
    inline ::std::vector<size_t>::const_iterator begin() const {return begin_ + rowIdx_.begin;}
    /**
     * Access a random access iterator referring to the past-the-end element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access iterator
     */
    inline ::std::vector<size_t>::const_iterator end() const {return begin_ + rowIdx_.end;}
    /**
     * Access a random access iterator pointing to the index of the first non-zero element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access iterator
     */
    inline ::std::vector<size_t>::iterator begin() {return begin_ + rowIdx_.begin;}
    /**
     * Access a random access iterator referring to the past-the-end element in the row.
     *  Since the matrix is sparse, the iterator can iterate only over index of non-zero element.
     *  \return random access iterator
     */
    inline ::std::vector<size_t>::iterator end() {return begin_ + rowIdx_.end;}
    
    /**
     * Access the value of one element in the row.
     *  \param  j Index of the accessed value
     *  \return Value of the element
     */
    inline bool test(size_t j) const {return std::binary_search(begin(), end(), j);}
    /**
     *  Access the index of the first non-zero element in a row.
     *  \return Index of the first non-zero element
     */
    inline size_t first() const {return end() - begin() == 0 ? -1 : *begin();}
    /**
     *  Access the number of non-zero elements in a row.
     *  \return Number of non-zero elements
     */
    inline size_t size() const {return rowIdx_.end - rowIdx_.begin;}
    /**
     *  Access the number of non-zero elements within a specified range of the row.
     *  \param  range begin and end index of considered elements
     *  \return Number of non-zero elements
     */
    inline size_t size(const size_t range[2]) const {return ::std::lower_bound(begin(), end(), range[1]) - ::std::lower_bound(begin(), end(), range[0]);}
    /**
     *  Access the number of non-zero elements within a specified range of the row.
     *  \param  range begin and end index of considered elements
     *  \return Number of non-zero elements
     */
    inline size_t size(const ::std::initializer_list<size_t> colRange) const {return size(colRange.begin());}
    /**
     *  \return True if the row contains no non-zero elements
     */
    inline bool empty() const {return (end() - begin()) == 0;}
    
    /**
     *  Adds a non-zero element in the row.
     *  The element's index must be bigger that any previous element.
     *  The object must be a valid reference to sufficiently allocated space.
     *  \param j Index of the element added.
     */
    inline void set(size_t j) {
      *end() = j;
      ++rowIdx_.end;
    }
    
    /**
     *  Access part of a row specified by an index range.
     *  This method returns a reference to a part of the bit row.
     *  \param  colRange  Range of the accessed bloc
     *  \return Reference to the bits in the bloc
     */
    inline ConstOffsetRowRef operator() (const ::std::initializer_list<size_t>& colRange) const {return (*this)(colRange.begin());}
    inline ConstOffsetRowRef operator() (const size_t colRange[2]) const;
    
    inline void swap(size_t a, size_t b);
    inline void move(size_t a, size_t b);
    
    /**
     *  Swaps the row referred by the object with another row.
     *  Both rows must be part of the same matrix.
     *  \param  b Row being swapped with the object.
     */
    void swap(RowRef b) {
      ::std::swap(rowIdx_, b.rowIdx_);
    }
    
  private:
    inline RowRef() = default;
    inline RowRef(::std::vector<size_t>::iterator begin, RowIdx& rowIdx) : begin_(begin), rowIdx_(rowIdx) {}
    
    ::std::vector<size_t>::iterator begin_;
    RowIdx& rowIdx_;
  };
  
  /**
   *  This class emulates a const ptr to a row.
   *  It contains a reference to the row.
   */
  class ConstRowPtr
  {
    friend class ConstIterator;
  public:
    inline ConstRowPtr(const ConstRowRef& row) : row_(row) {}
    inline const ConstRowRef* operator-> () const {return &row_;}
    
  private:
    ConstRowRef row_;
  };
  /**
   *  This class emulates a ptr to a row.
   *  It contains a reference to the row.
   */
  class RowPtr
  {
    friend class Iterator;
  public:
    inline RowPtr(const RowRef& row) : row_(row) {}
    inline RowRef* operator-> () {return &row_;}
    
  private:
    RowRef row_;
  };
  
  /**
   *  This is a random access input iterator of a SparseBitMatrix.
   *  It iterates over the matrix rows.
   */
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
    inline ConstIterator(::std::vector<size_t>::const_iterator begin, ::std::vector<RowIdx>::const_iterator rowIdx) :begin_(begin), rowIdx_(rowIdx) {}
    
    ::std::vector<size_t>::const_iterator begin_;
    ::std::vector<RowIdx>::const_iterator rowIdx_;
  };
  /**
   *  This is a random access iterator of a SparseBitMatrix.
   *  It iterates over the matrix rows.
   */
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
    inline Iterator(::std::vector<size_t>::iterator begin, ::std::vector<RowIdx>::iterator rowIdx) : begin_(begin), rowIdx_(rowIdx) {}
    
    ::std::vector<size_t>::iterator begin_;
    ::std::vector<RowIdx>::iterator rowIdx_;
  };
  
  SparseBitMatrix() = default;
  /**
   *  Copy constructor.
   */
  SparseBitMatrix(const SparseBitMatrix& b) {*this = b;}
  /**
   *  Copy constructor.
   */
  SparseBitMatrix(const BitMatrix& b) {*this = b;}
  /**
   *  Move constructor.
   */
  SparseBitMatrix(SparseBitMatrix&& b) {cols_ = b.cols_; ::std::swap(elementIdx_, b.elementIdx_); ::std::swap(rowIdx_, b.rowIdx_);}
  /**
   *  SparseBitMatrix constructor.
   *  Allocates space for the specified matrix structure.
   *  \param  rows Number of row
   *  \param  cols Number of columns
   *  \param  rowSizes Number of non-zero element within each row
   */
  inline SparseBitMatrix(size_t rows, size_t cols, size_t rowSizes) {resize(rows, cols, rowSizes);}
  /**
   *  SparseBitMatrix constructor.
   *  Allocates space for the specified matrix structure.
   *  \param  rowSizes Vector containing the number of non-zero element within each row
   *  \param  cols Number of columns
   */
  inline SparseBitMatrix(const ::std::vector<size_t>& rowSizes, size_t cols) {resize(rowSizes, cols);}
  
  inline SparseBitMatrix& operator = (const SparseBitMatrix& b);
  inline SparseBitMatrix& operator = (const fec::BitMatrix& b);
  /**
   *  Move assignement operator.
   */
  inline SparseBitMatrix& operator = (SparseBitMatrix&& b) {cols_ = b.cols_; ::std::swap(elementIdx_, b.elementIdx_); ::std::swap(rowIdx_, b.rowIdx_); return *this;}
  
  inline void resize(size_t rows, size_t cols, size_t rowSizes);
  inline void resize(const std::vector<size_t>& rowSizes, size_t cols);
  
  /**
   *  Access the number of rows in the object.
   *  \return Number of rows
   */
  inline size_t rows() const {return rowIdx_.size();}
  /**
   *  Access the number of columns in the object.
   *  \return Number of columns
   */
  inline size_t cols() const {return cols_;}
  /**
   *  Access the size of the object.
   *  We define the size as the number of non-zero elements.
   *  \return Number of non-zero elements
   */
  inline size_t size() const {return elementIdx_.size();}
  
  /**
   *  Access a submatrix specified by a row range and a column range.
   *  A copy of the data is performed.
   *  \param rowRange begin and end row index which the submatrix will contain.
   *  \param colRange begin and end column index which the submatrix will contain.
   *  \return submatrix
   */
  inline SparseBitMatrix operator() (const ::std::initializer_list<size_t>& rowRange, const ::std::initializer_list<size_t>& colRange) const {return (*this)(rowRange.begin(), colRange.begin());}
  inline SparseBitMatrix operator() (const size_t rowRange[2], const size_t colRange[2]) const;
  
  /**
   *  Access a random access input iterator pointing to the first row in the object.
   *  \return Random access input iterator
   */
  inline ConstIterator begin() const {return ConstIterator(elementIdx_.begin(), rowIdx_.begin());}
  /**
   *  Access a random access input iterator referring to the past-the-end row in the object.
   *  \return Random access input iterator
   */
  inline ConstIterator end() const {return ConstIterator(elementIdx_.begin(), rowIdx_.end());}
  /**
   *  Access a random access input iterator pointing to the first row in the object.
   *  \return Random access input iterator
   */
  inline ConstIterator cbegin() const {return ConstIterator(elementIdx_.begin(), rowIdx_.begin());}
  /**
   *  Access a random access input iterator referring to the past-the-end row in the object.
   *  \return Random access input iterator
   */
  inline ConstIterator cend() const {return ConstIterator(elementIdx_.begin(), rowIdx_.end());}
  /**
   *  Access a random access iterator pointing to the first row in the object.
   *  \return Random access iterator
   */
  inline Iterator begin() {return Iterator(elementIdx_.begin(), rowIdx_.begin());}
  /**
   *  Access a random access iterator referring to the past-the-end row in the object.
   *  \return Random access iterator
   */
  inline Iterator end() {return Iterator(elementIdx_.begin(), rowIdx_.end());}
  
  inline ConstRowRef operator[] (size_t i) const {return ConstRowRef(elementIdx_.begin() + rowIdx_[i].begin, elementIdx_.begin() + rowIdx_[i].end);}
  inline RowRef operator[] (size_t i) {return RowRef(elementIdx_.begin(), rowIdx_[i]);}
  inline size_t at(size_t i) const {return elementIdx_[i];}
  
  /**
   *  Computes the column sizes in the matrix.
   *  We define the column size as the number of non-zero elements
   *  within the specified range of a column.
   *  A column size is given only for the column within the specified range.
   *  \param[out] dst Vector containing the size of each column
   */
  inline void colSizes(const size_t rowRange[2], const size_t colRange[2], ::std::vector<size_t>& dst) const;
  /**
   *  Computes the column sizes in the matrix.
   *  We define the column size as the number of non-zero elements
   *  within the specified range of a column.
   *  A column size is given only for the column within the specified range.
   *  \param rowRange begin and end row index of considered elements
   *  \param colRange begin and end column index of considered elements
   *  \param[out] dst Vector containing the size of each column
   */
  inline void colSizes(::std::vector<size_t>& dst) const {colSizes({0, rows()}, {0, cols()}, dst);}
  /**
   *  Computes the column sizes in the matrix.
   *  We define the column size as the number of non-zero elements
   *  within the specified range of a column.
   *  A column size is given only for the column within the specified range.
   *  \param rowRange begin and end row index of considered elements
   *  \param colRange begin and end column index of considered elements
   *  \param[out] dst Vector containing the size of each column
   */
  inline void colSizes(const ::std::initializer_list<size_t>& rowRange, const ::std::initializer_list<size_t>& colRange, ::std::vector<size_t>& x) const {colSizes(rowRange.begin(), colRange.begin(), x);}
  
  /**
   *  Computes the row sizes in the matrix.
   *  We define the row size as the number of non-zero elements
   *  within the specified range of a row.
   *  A row size is given only for the row within the specified range.
   *  \param[out] dst Vector containing the size of each row
   */
  inline void rowSizes(::std::vector<size_t>& x) const {rowSizes({0, rows()}, {0, cols()}, x);}
  /**
   *  Computes the row sizes in the matrix.
   *  We define the row size as the number of non-zero elements
   *  within the specified range of a row.
   *  A row size is given only for the row within the specified range.
   *  \param rowRange begin and end row index of considered elements
   *  \param colRange begin and end column index of considered elements
   *  \param[out] dst Vector containing the size of each row
   */
  inline void rowSizes(const size_t rowRange[2], const size_t colRange[2], ::std::vector<size_t>& dst) const;
  /**
   *  Computes the row sizes in the matrix.
   *  We define the row size as the number of non-zero elements
   *  within the specified range of a row.
   *  A row size is given only for the row within the specified range.
   *  \param rowRange begin and end row index of considered elements
   *  \param colRange begin and end column index of considered elements
   *  \param[out] dst Vector containing the size of each row
   */
  inline void rowSizes(const ::std::initializer_list<size_t>& rowRange, const ::std::initializer_list<size_t>& colRange, ::std::vector<size_t>& dst) const {rowSizes(rowRange.begin(), colRange.begin(), dst);}
  
  inline void moveCol(size_t a, size_t b);
  
  /**
   *  Swaps two colums in the matrix.
   *  \param  a Index of the first column
   *  \param  b Index of the second column
   */
  inline void swapCols(size_t a, size_t b) {swapCols(a, b, {0, rows()});}
  inline void swapCols(size_t a, size_t b, const size_t rowRange[2]);
  /**
   *  Swaps a part of two colums in the matrix.
   *  \param  a Index of the first column
   *  \param  b Index of the second column
   *  \param  rowRange  Begin and end row involved in the operation.
   */
  inline void swapCols(size_t a, size_t b, const ::std::initializer_list<size_t>& rowRange) {swapCols(a, b, rowRange.begin());}
  
  inline SparseBitMatrix transpose() const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(cols_);
    ar & BOOST_SERIALIZATION_NVP(elementIdx_);
    ar & BOOST_SERIALIZATION_NVP(rowIdx_);
  }
  
  size_t cols_;
  ::std::vector<size_t> elementIdx_;
  ::std::vector<RowIdx> rowIdx_;
};
  
}

namespace std {
  inline void swap(fec::SparseBitMatrix::RowRef a, fec::SparseBitMatrix::RowRef b) {
    a.swap(b);
  }
}

namespace fec {

/**
 *  This class represents a full bit matrix.
 *  Every bit is stored in a compact BitField.
 */
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
  public:
    /**
     *  This is a random access input iterator of a BitMatrix::Row.
     *  It iterates over every elements in a row.
     */
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
      using namespace boost::serialization;
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
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(cols_);
    ar & BOOST_SERIALIZATION_NVP(rows_);
  }
  
  inline void resize(size_t rows, size_t cols);
  
  size_t cols_;
  std::vector<Row> rows_;
};
  
}


/**
 *  Access part of a row.
 *  This function returns a reference to a part of the bit row.
 *  \param  colRange  Range of the accessed bloc
 *  \return Reference to the bits in the bloc
 */
fec::SparseBitMatrix::ConstOffsetRowRef fec::SparseBitMatrix::RowRef::operator() (const size_t colRange[2]) const
{
  auto first = std::lower_bound(begin(), end(), colRange[0]);
  auto last = std::lower_bound(begin(), end(), colRange[1]);
  return ConstOffsetRowRef(first, last, colRange[0]);
}

/**
 *  Access part of a row specified by an index range.
 *  This method returns a reference to a part of the bit row.
 *  \param  colRange  Range of the accessed bloc
 *  \return Reference to the bits in the bloc
 */
fec::SparseBitMatrix::ConstOffsetRowRef fec::SparseBitMatrix::ConstRowRef::operator() (const size_t colRange[2]) const
{
  auto first = std::lower_bound(begin(), end(), colRange[0]);
  auto last = std::lower_bound(begin(), end(), colRange[1]);
  return ConstOffsetRowRef(first, last, colRange[0]);
}

/**
 *  Swaps two bits in a row.
 *  \param  a Index of the first bit
 *  \param  b Index of the second bit
 */
void fec::SparseBitMatrix::RowRef::swap(size_t a, size_t b)
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

/**
 *  Move one bits from a position the another one.
 *  All the bits in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
void fec::SparseBitMatrix::RowRef::move(size_t a, size_t b)
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

/**
 *  Assignment operator.
 *  The method reorders data (if needed) to optimize access performance
 *  \param  b Bit matrix being copied
 */
fec::SparseBitMatrix& fec::SparseBitMatrix::operator = (const fec::SparseBitMatrix& b)
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

/**
 *  Assignment operator.
 *  The method reorders data (if needed) to optimize access performance
 *  \param  b Bit matrix being copied
 */
fec::SparseBitMatrix& fec::SparseBitMatrix::operator = (const fec::BitMatrix& b)
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

/**
 *  Access a submatrix specified by a row range and a column range.
 *  A copy of the data is performed.
 *  \param rowRange begin and end row index which the submatrix will contain.
 *  \param colRange begin and end column index which the submatrix will contain.
 *  \return submatrix
 */
fec::SparseBitMatrix fec::SparseBitMatrix::operator() (const size_t rowRange[2], const size_t colRange[2]) const
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

void fec::SparseBitMatrix::colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
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

void fec::SparseBitMatrix::rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
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
void fec::SparseBitMatrix::moveCol(size_t a, size_t b)
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
void fec::SparseBitMatrix::swapCols(size_t a, size_t b, const size_t rowRange[2])
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
fec::SparseBitMatrix fec::SparseBitMatrix::transpose() const
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

/**
 *  Resize the matrix to a specified shape.
 *  Data in the matrix may become invalid.
 *  \param  rows Number of row
 *  \param  cols Number of columns
 *  \param  rowSizes Number of non-zero element within each row
 */
void fec::SparseBitMatrix::resize(size_t rows, size_t cols, size_t rowSizes)
{
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
void fec::SparseBitMatrix::resize(const std::vector<size_t>& rowSizes, size_t cols)
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



/**
 *  Access the index of the first non-zero element in a row.
 *  \return Index of the first non-zero element
 */
size_t fec::BitMatrix::Row::first() const
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
size_t fec::BitMatrix::Row::size(const size_t range[2]) const
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
void fec::BitMatrix::Row::swap(size_t a, size_t b)
{
  bool aVal = test(a);
  bool bVal = test(b);
  bool tmp = aVal ^ bVal;
  set(a, aVal ^ tmp);
  set(b, bVal ^ tmp);
}

/**
 *  Move one bits from a position the another one.
 *  All the bits in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
void fec::BitMatrix::Row::move(size_t a, size_t b)
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

/**
 *  Access part of a row.
 *  This function returns a reference to a part of the bit row.
 *  \param  colRange  Range of the accessed bloc
 *  \return Reference to the bits in the bloc
 */
fec::BitMatrix::Row fec::BitMatrix::Row::operator() (const size_t colRange[2]) const
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

/**
 *  Addition assignement operator.
 *  This function compute implace the modulo-2 addition between 2 rows.
 *  \param  b Row being added the the member
 */
void fec::BitMatrix::Row::operator += (const Row& b)
{
  for (size_t i = 0; i < elements_.size(); ++i) {
    elements_[i] ^= b.elements_[i];
  }
}

/**
 *  Addition assignement operator.
 *  This function compute implace the modulo-2 addition between 2 rows.
 *  \param  b Row being added the the member
 */
void fec::BitMatrix::Row::operator += (SparseBitMatrix::ConstRowRef b)
{
  for (auto bRow = b.begin(); bRow < b.end(); ++bRow) {
    elements_[*bRow/blocSize()].toggle(*bRow%blocSize());
  }
}

fec::BitMatrix::BitMatrix(const SparseBitMatrix& b)
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

/**
 *  Access a submatrix specified by a row range and a column range.
 *  A copy of the data is performed.
 *  \param rowRange begin and end row index which the submatrix will contain.
 *  \param colRange begin and end column index which the submatrix will contain.
 *  \return submatrix
 */
fec::BitMatrix fec::BitMatrix::operator() (const size_t rowRange[2], const size_t colRange[2]) const
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
void fec::BitMatrix::colSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& x) const
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
void fec::BitMatrix::rowSizes(const size_t rowRange[2], const size_t colRange[2], std::vector<size_t>& dst) const
{
  dst.resize(rowRange[1] - rowRange[0]);
  for (size_t i = rowRange[0]; i < rowRange[1]; ++i) {
    dst[i] = rows_[i].size(colRange);
  }
}

/**
 *  Move one column from a position the another one.
 *  All the columns in between are shifted by one to fill the old position.
 *  \param  a Index of source
 *  \param  b Index of destination
 */
void fec::BitMatrix::moveCol(size_t a, size_t b)
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
void fec::BitMatrix::resize(size_t rows, size_t cols)
{
  cols_ = cols;
  rows_.resize(rows);
  for (auto row = begin(); row < end(); ++row) {
    row->resize(cols_);
  }
}

inline std::ostream& operator<<(std::ostream& os, const fec::BitMatrix& matrix)
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

inline std::ostream& operator<<(std::ostream& os, const fec::SparseBitMatrix& matrix)
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
