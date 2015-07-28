/*******************************************************************************
 *  \file Interleaver.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-17
 *  \version Last update : 2015-06-17
 *
 *  Declaration of the Interleaver class
 ******************************************************************************/

#ifndef INTERLEAVER_H
#define INTERLEAVER_H

#include <stdint.h>

#include <boost/container/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

namespace fec {

class Interleaver {
  friend class boost::serialization::access;
public:
  Interleaver() = default;
  Interleaver(::boost::container::vector<size_t> sequence) {
    sequence_ = sequence;
    dstSize_ = *std::max_element(sequence_.begin(), sequence_.end()) + 1;
    srcSize_ = sequence_.size();
  }
  Interleaver(::boost::container::vector<size_t> sequence, size_t srcSize, size_t dstSize) {
    sequence_ = sequence;
    dstSize_ = dstSize;
    srcSize_ = srcSize;
  }
  
  size_t& dstSize() {return dstSize_;}
  size_t& srcSize() {return srcSize_;}
  size_t srcSize() const {return srcSize_;}
  size_t dstSize() const {return dstSize_;}
  
  template <typename T> void interleave(const ::boost::container::vector<T>& input, ::boost::container::vector<T>& output) const;
  template <typename T> void deInterleave(const ::boost::container::vector<T>& input, ::boost::container::vector<T>& output) const;
  
  template <typename T> ::boost::container::vector<T> interleave(const ::boost::container::vector<T>& input) const {
    ::boost::container::vector<T> output;
    interleave(input, output);
    return output;
  }
  template <typename T> ::boost::container::vector<T> deInterleave(const ::boost::container::vector<T>& input) const {
    ::boost::container::vector<T> output;
    deInterleave<T>(input, output);
    return output;
  }
  
  template <typename T> void interleaveNBloc(typename ::boost::container::vector<T>::const_iterator input, typename ::boost::container::vector<T>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      interleaveBloc<T>(input, output);
      input += srcSize();
      output += dstSize();
    }
  }
  
  template <typename T> void deInterleaveNBloc(typename ::boost::container::vector<T>::const_iterator input, typename ::boost::container::vector<T>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      deInterleaveBloc<T>(input, output);
      input += dstSize();
      output += srcSize();
    }
  }
  
  template <typename T> void interleaveBloc(typename ::boost::container::vector<T>::const_iterator input, typename ::boost::container::vector<T>::iterator output) const;
  template <typename T> void deInterleaveBloc(typename ::boost::container::vector<T>::const_iterator input, typename ::boost::container::vector<T>::iterator output) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(sequence_);
  }
  
  ::boost::container::vector<size_t> sequence_;
  size_t dstSize_;
  size_t srcSize_;
};
  
}

template <typename T>
void fec::Interleaver::interleave(const boost::container::vector<T>& input, boost::container::vector<T>& output) const
{
  output.resize(input.size() / srcSize() * dstSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    interleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void fec::Interleaver::deInterleave(const boost::container::vector<T>& input, boost::container::vector<T>& output) const
{
  output.resize(input.size() / dstSize() * srcSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    deInterleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void fec::Interleaver::interleaveBloc(typename boost::container::vector<T>::const_iterator input, typename boost::container::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[sequence_[i]] = input[i];
  }
}

template <typename T>
void fec::Interleaver::deInterleaveBloc(typename boost::container::vector<T>::const_iterator input, typename boost::container::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[i] = input[sequence_[i]];
  }
}

#endif