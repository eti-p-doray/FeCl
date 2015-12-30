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
    template <class T, template <class> class LogSumAlg>
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
      
      template <class U = typename LogSumAlg<T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      template <class U = typename LogSumAlg<T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      
      std::vector<T> distance_;
      std::vector<T> metric_;
      std::vector<T> max_;
      
      LogSumAlg<T> logSum_;
      
      Modulation::Structure structure_;
    };
    
    /**
     *  Constructor.
     *  Allocates metric buffers based on the given code structure.
     *  \param  codeStructure Convolutional code structure describing the code
     */
    template <class T, template <class> class LogSumAlg>
    Demodulator<T, LogSumAlg>::Demodulator(const Modulation::Structure& structure) : structure_(structure)
    {
      distance_.resize(this->structure().constellation().size() / this->structure().dimension());
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
    template <class T, template <class> class LogSumAlg>
    template <class InputIterator, class OutputIterator>
    void Demodulator<T, LogSumAlg>::soDemodulate(Modulation::iterator<InputIterator> input, T k, OutputIterator word)
    {
      for (size_t i = 0; i < structure().length(); ++i) {
        distanceUpdate(input);
        aPosterioriUpdate(input, k, word);
        input.at(Modulation::Symbol) += structure().symbolWidth();
        input.at(Modulation::Word) += structure().size();
        word += structure().size();
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class InputIterator>
    void Demodulator<T, LogSumAlg>::distanceUpdate(Modulation::iterator<InputIterator> input)
    {
      auto symbol = input.at(Modulation::Symbol);
      auto word = input.at(Modulation::Word);
      auto distance = distance_.begin();
      for (auto j = structure().constellation().begin(); j != structure().constellation().end(); j += structure().dimension(), ++distance) {
        *distance = -sqDistance(symbol, j, structure().dimension());
      }
      if (input.count(Modulation::Word)) {
        for (BitField<size_t> j = 0; j < structure().constellation().size(); j += structure().dimension(), ++distance) {
          *distance += accumulate(j, word, structure().dimension());
        }
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class InputIterator, class OutputIterator>
    void Demodulator<T, LogSumAlg>::aPosterioriUpdate(Modulation::iterator<InputIterator> input, T factor, OutputIterator output)
    {
      auto wordIn = input.at(Modulation::Word);
      auto wordOut = output;
      
      for (size_t j = 0; j < structure().size(); j+=structure().wordWidth()) {
        wordUpdateImpl(j);
        
        if (input.count(Modulation::Word)) {
          for (size_t k = 1; k < structure().wordWidth(); ++k) {
            wordOut[k] = factor * structure().scalingFactor() * (metric_[k] - metric_[0] - wordIn[k]);
          }
        }
        else {
          for (size_t k = 1; k < structure().wordWidth(); ++k) {
            wordOut[k] = factor * structure().scalingFactor() * (metric_[k] - metric_[0]);
          }
        }
        wordIn += structure().wordWidth();
        wordOut += structure().wordWidth();
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class U, typename std::enable_if<U::value>::type*>
    void Demodulator<T, LogSumAlg>::wordUpdateImpl(size_t j)
    {
      std::fill(metric_.begin(), metric_.end(), -std::numeric_limits<T>::infinity());
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        metric_[input.test(j, structure.wordCount())] = logSum_.sum(metric_[input.test(j, structure.wordCount())], distance_[input]);
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class U, typename std::enable_if<!U::value>::type*>
    void Demodulator<T, LogSumAlg>::wordUpdateImpl(size_t j)
    {
      std::fill(max_.begin(), max_.end(), -std::numeric_limits<T>::infinity());
      std::fill(metric_.begin(), metric_.end(), T(0));
      
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        max_[input.test(j, structure.wordCount())] = logSum_.max(distance_[input], max[input.test(j, structure.wordCount())]);
      }
      for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
        metric_[input.test(j, structure.wordCount())] = logSum_.sum(logSum_.prior(distance_[input], max_[input.test(j, structure.wordCount())]), metric_[input.test(j, structure.wordCount())]);
      }
      for (size_t i = 0; i < structure().wordCount(); ++i) {
        metric_[i] = logSum_.post(metric_[i], max_[i])
      }
    }
    
  }
  
}

#endif
