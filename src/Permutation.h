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

#ifndef FEC_PERMUTATION_H
#define FEC_PERMUTATION_H

#include <stdint.h>

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

namespace fec {
  
  /**
   * This class represents a permutation vector.
   * A permutation generates a sequence of output data where each element
   * is picked at a specific index from the input sequence.
   * The index is defined by the index sequence given at the construction.
   * The permutation can permute many independant sequences at once.
   */
  class Permutation {
    friend class boost::serialization::access;
  public:
    Permutation() = default;
    /**
     * Permutation constructor.
     *  The size of the input and output sequence is automatically detected
     *  from the given index sequence
     *  \param  sequence  Index sequence specifying the source index of each output element.
     */
    inline Permutation(const std::vector<size_t>& sequence);
    /**
     * Permutation constructor.
     *  You can specify a custom input and output seuqence size.
     *  Be carefull specifying these size because an invalid index in the index sequence
     *  will result in segfault.
     *  \param  sequence  Index sequence specifying the source index of each output element.
     *  \param  inputSize Length of the input sequence for interleaving.
     */
    inline Permutation(::std::vector<size_t> sequence, size_t inputSize);

    size_t inputSize() const {return inputSize_;}
    size_t outputSize() const {return sequence_.size();}
    
    size_t operator[] (size_t i) const {return sequence_[i];}
    
    template <typename Vector> void permute(const Vector& input, size_t width, Vector& output) const;
    template <typename Vector> void depermute(const Vector& input, size_t width, Vector& output) const;
    
    template <typename Vector> Vector permute(const Vector& input, size_t width = 1) const;
    template <typename Vector> Vector depermute(const Vector& input, size_t width = 1) const;
    
    template <class InputIterator, class OutputIterator>
    void permuteBlocks(InputIterator inputf, InputIterator inputl, size_t width, OutputIterator output) const;
    template <class InputIterator, class OutputIterator>
    void depermuteBlocks(InputIterator inputf, InputIterator inputl, size_t width, OutputIterator output) const;
    
    template <class InputIterator, class OutputIterator>
    void permuteBlock(InputIterator input, size_t width, OutputIterator output) const;
    template <class InputIterator, class OutputIterator>
    void depermuteBlock(InputIterator input, size_t width, OutputIterator output) const;

  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar & BOOST_SERIALIZATION_NVP(sequence_);
      ar & BOOST_SERIALIZATION_NVP(inputSize_);
    }
    
    std::vector<size_t> sequence_;
    size_t inputSize_ = 0;
  };
  
}


fec::Permutation::Permutation(const std::vector<size_t>& sequence) {
  if (sequence.size() == 0) {
    return;
  }
  sequence_ = sequence;
  inputSize_ = *std::max_element(sequence_.begin(), sequence_.end()) + 1;
}

fec::Permutation::Permutation(::std::vector<size_t> sequence, size_t inputSize) {
  sequence_ = sequence;
  inputSize_ = inputSize;
}

template <typename Vector>
void fec::Permutation::permute(const Vector& input, size_t width, Vector& output) const
{
  output.resize(input.size() / inputSize() * outputSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += inputSize(), outputIt += outputSize()) {
    permuteBlock(inputIt, width, outputIt);
  }
}

template <typename Vector>
void fec::Permutation::depermute(const Vector& input, size_t width, Vector& output) const
{
  output.resize(input.size() / outputSize() * inputSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += outputSize(), outputIt += inputSize()) {
    depermuteBlock(inputIt, width, outputIt);
  }
}

template <typename Vector>
Vector fec::Permutation::permute(const Vector& input, size_t width) const
{
  Vector output;
  permute(input, width, output);
  return output;
}

template <typename Vector>
Vector fec::Permutation::depermute(const Vector& input, size_t width) const
{
  Vector output;
  depermute(input, width, output);
  return output;
}

template <class InputIterator, class OutputIterator>
void fec::Permutation::permuteBlocks(InputIterator inputf, InputIterator inputl, size_t width, OutputIterator output) const
{
  while (inputf != inputl) {
    permuteBlock(inputf, output);
    inputf += inputSize();
    output += outputSize();
  }
}

template <class InputIterator, class OutputIterator>
void fec::Permutation::depermuteBlocks(InputIterator inputf, InputIterator inputl, size_t width, OutputIterator output) const
{
  while (inputf != inputl) {
    dePermuteBlock(inputf, output);
    inputf += inputSize();
    output += outputSize();
  }
}

template <class InputIterator, class OutputIterator>
void fec::Permutation::permuteBlock(InputIterator input, size_t width, OutputIterator output) const
{
  for (size_t i = 0; i < sequence_.size(); ++i) {
    std::copy(input+sequence_[i]*width, input+(sequence_[i]+1)*width, output+i*width);
  }
}

template <class InputIterator, class OutputIterator>
void fec::Permutation::depermuteBlock(InputIterator input, size_t width, OutputIterator output) const
{
  for (size_t i = 0; i < sequence_.size(); ++i) {
    std::copy(input+i*width, input+(i+1)*width, output+sequence_[i]*width);
  }
}

#endif