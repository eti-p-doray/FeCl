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

#include <vector>
#include <stdint.h>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

class Interleaver {
  friend class boost::serialization::access;
public:
  Interleaver() = default;
  Interleaver(std::vector<size_t> sequence) {
    sequence_ = sequence;
  }
  
  size_t size() const {return sequence_.size();}
  
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
      input += size();
      output += size();
    }
  }
  
  template <typename T> void deInterleaveNBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      deInterleaveBloc<T>(input, output);
      input += size();
      output += size();
    }
  }
  
  template <typename T> void interleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const;
  
  template <typename T> void deInterleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const;
  
  size_t serialSize() const
  {
    return ( sequence_.size() + 1 ) * sizeof(size_t);
  }
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(sequence_);
  }
  
  std::vector<size_t> sequence_;
};

template <typename T>
void Interleaver::interleave(const std::vector<T>& input, std::vector<T>& output) const
{
  output.resize(input.size());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    interleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void Interleaver::deInterleave(const std::vector<T>& input, std::vector<T>& output) const
{
  output.resize(input.size());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    deInterleaveBloc<T>(inputIt, outputIt);
  }
}

template <typename T>
void Interleaver::interleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[sequence_[i]] = input[i];
  }
}

template <typename T>
void Interleaver::deInterleaveBloc(typename std::vector<T>::const_iterator input, typename std::vector<T>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[i] = input[sequence_[i]];
  }
}

#endif