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

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

namespace fec {

class Interleaver {
  friend class boost::serialization::access;
public:
  Interleaver() = default;
  Interleaver(::std::vector<size_t> sequence) {
    sequence_ = sequence;
    srcSize_ = *std::max_element(sequence_.begin(), sequence_.end()) + 1;
    dstSize_ = sequence_.size();
  }
  Interleaver(::std::vector<size_t> sequence, size_t srcSize, size_t dstSize) {
    sequence_ = sequence;
    dstSize_ = dstSize;
    srcSize_ = srcSize;
  }
  
  size_t& dstSize() {return dstSize_;}
  size_t& srcSize() {return srcSize_;}
  size_t srcSize() const {return srcSize_;}
  size_t dstSize() const {return dstSize_;}
  
  size_t operator[] (size_t i) const {return sequence_[i];}
  
  template <typename T> void interleave(const std::vector<T>& input, std::vector<T>& output) const;
  template <typename T> void deInterleave(const std::vector<T>& input, std::vector<T>& output) const;
  
  template <typename T> std::vector<T> interleave(const std::vector<T>& input) const {
    std::vector<T> output;
    interleave(input, output);
    return output;
  }
  template <typename T> std::vector<T> deInterleave(const std::vector<T>& input) const {
    std::vector<T> output;
    deInterleave<T>(input, output);
    return output;
  }
  
  template <typename T> void interleaveNBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      interleaveBloc<T>(input, output);
      input += srcSize();
      output += dstSize();
    }
  }
  
  template <typename T> void deInterleaveNBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      deInterleaveBloc<T>(input, output);
      input += dstSize();
      output += srcSize();
    }
  }
  
  template <typename T> void interleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const;
  template <typename T> void deInterleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(sequence_);
  }
  
  std::vector<size_t> sequence_;
  size_t dstSize_;
  size_t srcSize_;
};
  
}

template <typename T>
void fec::Interleaver::interleave(const std::vector<T>& input, std::vector<T>& output) const
{
  output.resize(input.size() / srcSize() * dstSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    interleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void fec::Interleaver::deInterleave(const std::vector<T>& input, std::vector<T>& output) const
{
  output.resize(input.size() / dstSize() * srcSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    deInterleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void fec::Interleaver::interleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[i] = input[sequence_[i]];
  }
}

template <typename T>
void fec::Interleaver::deInterleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[sequence_[i]] = input[i];
  }
}

#endif