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

#ifndef FEC_DEMODULATOR_H
#define FEC_DEMODULATOR_H

#include <vector>
#include <memory>

#include "Modulation.h"
#include "LlrMetrics.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the abstract implementation of the map decoder.
     *  This algorithm is used for decoding with a-priori information
     *  in a ConvolutionalCodec.
     *  The reason for this class is to offer an common interface of map decoders
     *  while allowing the compiler to inline implementation specific functions
     *  by using templates instead of polymorphism.
     */
    template <DecoderAlgorithm algorithm, class T>
    class Demodulator
    {
    public:
      Demodulator(const Modulation::Structure&); /**< Constructor */
      virtual ~Demodulator() = default; /**< Default destructor */
      
      template <class InputIterator, class OutputIterator>
      void soDemodulate(Modulation::iterator<InputIterator> input, T k, OutputIterator word);
      
    private:
      inline const Modulation::Structure& structure() const {return structure_;} /**< Access the code structure */
      
      template <class InputIterator>
      void distanceUpdate(Modulation::iterator<InputIterator> input);/**< distance metric calculation. */
      template <class InputIterator, class OutputIterator>
      void aPosterioriUpdate(Modulation::iterator<InputIterator> input, T k, OutputIterator output);/**< Final (msg) L-values calculation. */
      
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      
      std::vector<T> distance_;
      std::vector<T> metric_;
      std::vector<T> max_;
      
      LogSum<algorithm,T> logSum_;
      
      Modulation::Structure structure_;
    };
    
    /**
     *  Constructor.
     *  Allocates metric buffers based on the given code structure.
     *  \param  codeStructure Convolutional code structure describing the code
     */
    template <DecoderAlgorithm algorithm, class T>
    Demodulator<algorithm, T>::Demodulator(const Modulation::Structure& structure) : structure_(structure)
    {
      distance_.resize(this->structure().constellation().size() / this->structure().dimension());
      metric_.resize(this->structure().wordCount());
      if (!LogSum<algorithm,T>::isRecursive::value) {
        max_.resize(this->structure().wordCount());
      }
    }
    
    /**
     *  Decodes one blocs of information bits.
     *  A posteriori information about the msg is output instead of the decoded bit sequence.
     *  \param  parityIn  Input iterator pointing to the first element
     *    in the parity L-value sequence
     *  \param  messageOut[out] Output iterator pointing to the first element
     *    in the a posteriori information L-value sequence.
     *    Output needs to be pre-allocated.
     */
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator, class OutputIterator>
    void Demodulator<algorithm, T>::soDemodulate(Modulation::iterator<InputIterator> input, T k, OutputIterator word)
    {
      for (size_t i = 0; i < structure().length(); ++i) {
        distanceUpdate(input);
        aPosterioriUpdate(input, k, word);
        input.at(Modulation::Symbol) += structure().symbolWidth();
        input.at(Modulation::Word) += structure().size()*(structure().wordCount()-1);
        word += structure().size()*(structure().wordCount()-1);
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator>
    void Demodulator<algorithm, T>::distanceUpdate(Modulation::iterator<InputIterator> input)
    {
      auto symbol = input.at(Modulation::Symbol);
      auto word = input.at(Modulation::Word);
      auto distance = distance_.begin();
      for (auto j = structure().constellation().begin(); j != structure().constellation().end(); j += structure().dimension(), ++distance) {
        *distance = -sqDistance(symbol, j, structure().dimension());
      }
      if (input.count(Modulation::Word)) {
        for (BitField<size_t> j = 0; j < structure().constellation().size(); j += structure().dimension(), ++distance) {
          *distance += mergeMetrics(word, 1, structure().dimension(), j);
        }
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator, class OutputIterator>
    void Demodulator<algorithm, T>::aPosterioriUpdate(Modulation::iterator<InputIterator> input, T factor, OutputIterator word)
    {
      auto wordIn = input.at(Modulation::Word);
      auto wordOut = word;
      
      for (size_t i = 0; i < structure().size(); i+=structure().wordWidth()) {
        wordUpdateImpl(i);
        
        if (input.count(Modulation::Word)) {
          for (size_t j = 1; j < structure().wordCount(); ++j) {
            wordOut[j-1] = factor * structure().scalingFactor() * (metric_[j] - metric_[0] - wordIn[j-1]);
          }
        }
        else {
          for (size_t j = 1; j < structure().wordCount(); ++j) {
            wordOut[j-1] = factor * structure().scalingFactor() * (metric_[j] - metric_[0]);
          }
        }
        wordIn += structure().wordCount()-1;
        wordOut += structure().wordCount()-1;
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<U::value>::type*>
    void Demodulator<algorithm, T>::wordUpdateImpl(size_t j)
    {
      std::fill(metric_.begin(), metric_.end(), -std::numeric_limits<T>::infinity());
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        metric_[input.test(j, structure().wordWidth())] = logSum_(metric_[input.test(j, structure().wordWidth())], distance_[input]);
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<!U::value>::type*>
    void Demodulator<algorithm, T>::wordUpdateImpl(size_t j)
    {
      std::fill(max_.begin(), max_.end(), -std::numeric_limits<T>::infinity());
      std::fill(metric_.begin(), metric_.end(), T(0));
      
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        max_[input.test(j, structure().wordWidth())] = std::max(distance_[input], max_[input.test(j, structure().wordWidth())]);
      }
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        metric_[input.test(j, structure().wordWidth())] = logSum_(logSum_.prior(distance_[input], max_[input.test(j, structure().wordWidth())]), metric_[input.test(j, structure().wordWidth())]);
      }
      for (size_t i = 0; i < structure().wordCount(); ++i) {
        metric_[i] = logSum_.post(metric_[i], max_[i]);
      }
    }
    
  }
  
}

#endif
