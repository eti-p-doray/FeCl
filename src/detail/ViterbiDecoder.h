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

#ifndef FEC_VITERBI_DECODER_H
#define FEC_VITERBI_DECODER_H

#include <vector>
#include <memory>

#include "Convolutional.h"
#include "LlrMetrics.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the implementation of the viterbi decoder.
     *  This algorithm is used for simple decoding in a ConvolutionalCodec.
     */
    template <class T>
    class ViterbiDecoder
    {
    public:
      ViterbiDecoder(const Convolutional::Structure&);
      ViterbiDecoder(const ViterbiDecoder&) = default;
      ~ViterbiDecoder() = default;
      
      template <class InputIterator, class OutputIterator>
      void decode(Codec::iterator<InputIterator> input, OutputIterator msg);
      
    private:
      inline const Convolutional::Structure& structure() const {return structure_;}
      
      std::vector<T> previousPath_;
      std::vector<T> nextPath_;
      std::vector<T> branch_;
      std::vector<BitField<size_t>> stateTraceBack_;
      std::vector<BitField<size_t>> inputTraceBack_;
      
      Convolutional::Structure structure_;
    };
    
    /**
     *  Decodes one bloc of information bits.
     *  \param  parityIn  Input iterator pointing to the first element
     *    in the parity L-value sequence
     *  \param  messageOut[out] Output iterator pointing to the first element
     *    in the decoded msg sequence.
     *    Output needs to be pre-allocated.
     */
    template <class T>
    template <class InputIterator, class OutputIterator>
    void ViterbiDecoder<T>::decode(Codec::iterator<InputIterator> input, OutputIterator msg)
    {
      previousPath_[0] = 0;
      std::fill(previousPath_.begin()+1, previousPath_.end(), -std::numeric_limits<T>::infinity());
      auto stateTraceBack = stateTraceBack_.begin();
      auto inputTraceBack = inputTraceBack_.begin();
      auto parity = input.at(Codec::Parity);
      auto syst = input.at(Codec::Syst);
      
      for (size_t i = 0; i < structure().length() + structure().tailSize(); ++i) {
        std::fill(nextPath_.begin(), nextPath_.end(), -std::numeric_limits<T>::infinity());
        
        for (BitField<size_t> j = 0; j < structure().trellis().outputCount(); ++j) {
          branch_[j] = mergeMetrics(parity, structure().parityWidth(), structure().trellis().outputWidth(), j);
        }
        if (input.count(Codec::Syst)) {
          for (BitField<size_t> j = 0; j < structure().trellis().inputCount(); ++j) {
            branch_[j] = mergeMetrics(syst, structure().systWidth(), structure().trellis().inputWidth(), j);
          }
        }
        parity += structure().outputLength();
        syst += structure().inputLength();
        
        auto previousPath = previousPath_.begin();
        auto state = structure().trellis().beginState();
        auto output = structure().trellis().beginOutput();
        for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
          for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
            auto & pathMetricRef = nextPath_[state[k]];
            T competitor = *previousPath + branch_[output[k]];
            if (competitor >= pathMetricRef) {
              stateTraceBack[state[k]] = j;
              inputTraceBack[state[k]] = k;
              pathMetricRef = competitor;
            }
          }
          state+=structure().trellis().inputCount();
          output+=structure().trellis().inputCount();
          previousPath++;
        }
        stateTraceBack += structure().trellis().stateCount();
        inputTraceBack += structure().trellis().stateCount();
        
        T max = -std::numeric_limits<T>::infinity();
        for (auto nextPathMetric = nextPath_.begin(); nextPathMetric < nextPath_.end(); nextPathMetric++) {
          if (*nextPathMetric > max) {
            max = *nextPathMetric;
          }
        }
        for (auto nextPathMetric = nextPath_.begin(); nextPathMetric < nextPath_.end(); nextPathMetric++) {
          *nextPathMetric -= max;
        }
        swap(previousPath_, nextPath_);
      }
      
      stateTraceBack -= structure().trellis().stateCount();
      inputTraceBack -= structure().trellis().stateCount();
      
      BitField<size_t> bestState = 0;
      switch (structure().termination()) {
        case Trellis::Truncate:
          for (BitField<size_t> i = 0; i < structure().trellis().stateCount(); ++i) {
            if (previousPath_[i] > previousPath_[bestState]) {
              bestState = i;
            }
          }
          break;
          
        default:
        case Trellis::Tail:
          break;
      }
      
      msg += (structure().length() - 1) * structure().inputLength();
      for (int64_t i = structure().length() + structure().tailLength() - 1; i >= 0; --i) {
        if (i < structure().length()) {
          for (BitField<size_t> j = 0; j < structure().trellis().inputWidth(); j += structure().msgWidth()) {
            msg[j] = inputTraceBack[bestState].test(j, structure().msgWidth());
          }
          msg -= structure().inputLength();
        }
        bestState = stateTraceBack[bestState];
        stateTraceBack -= structure().trellis().stateCount();
        inputTraceBack -= structure().trellis().stateCount();
      }
    }
    
    /**
     *  Constructor.
     *  Allocates metric buffers based on the given code structure.
     *  \param  codeStructure Convolutional code structure describing the code
     */
    template <class T>
    ViterbiDecoder<T>::ViterbiDecoder(const Convolutional::Structure& structure) : structure_(structure)
    {
      nextPath_.resize(structure.trellis().stateCount());
      previousPath_.resize(structure.trellis().stateCount());
      stateTraceBack_.resize((structure.length()+structure.tailLength())*structure.trellis().stateCount());
      inputTraceBack_.resize((structure.length()+structure.tailLength())*structure.trellis().stateCount());
      branch_.resize(structure.trellis().outputCount());
    }
    
  }
  
}

#endif
