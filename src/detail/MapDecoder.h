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

#ifndef FEC_MAP_DECODER_H
#define FEC_MAP_DECODER_H

#include <vector>
#include <memory>

#include "Convolutional.h"
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
    class MapDecoder
    {
    public:
      MapDecoder(const Convolutional::Structure&); /**< Constructor */
      MapDecoder(const MapDecoder&) = default;
      virtual ~MapDecoder() = default; /**< Default destructor */
      
      template <class InputIterator, class OutputIterator>
      void soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output);
      
      void setScalingFactor(double factor) {structure_.setScalingFactor(factor);}
      
    private:
      inline const Convolutional::Structure& structure() const {return structure_;} /**< Access the code structure */
      
      template <class InputIterator>
      void branchUpdate(Codec::iterator<InputIterator> input);/**< Branch metric calculation. */
      void forwardUpdate();/**< Forward metric calculation. */
      void backwardUpdate();/**< Backard metric calculation. */
      template <class InputIterator, class OutputIterator>
      void aPosterioriUpdate(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output);/**< Final (msg) L-values calculation. */
      
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void forwardUpdateImpl(typename std::vector<T>::iterator forwardMetric, typename std::vector<T>::const_iterator branchMetric);/**< Forward metric calculation. */
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void forwardUpdateImpl(typename std::vector<T>::iterator forwardMetric, typename std::vector<T>::const_iterator branchMetric);/**< Forward metric calculation. */
      
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void backwardUpdateImpl(typename std::vector<T>::iterator backwardMetric, typename std::vector<T>::const_iterator branchMetric);/**< Forward metric calculation. */
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void backwardUpdateImpl(typename std::vector<T>::iterator backwardMetric, typename std::vector<T>::const_iterator branchMetric);/**< Forward metric calculation. */
      
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void msgUpdateImpl(typename std::vector<T>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void msgUpdateImpl(typename std::vector<T>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
      
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      void parityUpdateImpl(typename std::vector<T>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
      template <class U = typename LogSum<algorithm,T>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      void parityUpdateImpl(typename std::vector<T>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
      
      std::vector<T> buffer_;
      std::vector<T> branch_;/**< Branch metric buffer (gamma) */
      std::vector<T> forward_;/**< Forward metric buffer (alpha) */
      std::vector<T> backward_;/**< Backard metric buffer (beta) */
      
      LogSum<algorithm,T> logSum_;
      
      Convolutional::Structure structure_;
    };
    
    /**
     *  Constructor.
     *  Allocates metric buffers based on the given code structure.
     *  \param  codeStructure Convolutional code structure describing the code
     */
    template <DecoderAlgorithm algorithm, class T>
    MapDecoder<algorithm, T>::MapDecoder(const Convolutional::Structure& structure) : structure_(structure)
    {
      branch_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().inputCount()*this->structure().trellis().stateCount());
      forward_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().stateCount());
      backward_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().stateCount());
      
      buffer_.resize(std::max(this->structure().trellis().outputCount(), this->structure().trellis().inputCount()));
      if (!LogSum<algorithm,T>::isRecursive::value) {
        buffer_.resize(std::max(buffer_.size(), this->structure().trellis().stateCount()*(this->structure().trellis().inputCount()+1)));
        buffer_.resize(std::max(buffer_.size(), 2*std::max(this->structure().msgCount(), this->structure().parityCount())));
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
    void MapDecoder<algorithm, T>::soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output)
    {
      branchUpdate(input);
      forwardUpdate();
      backwardUpdate();
      aPosterioriUpdate(input, output);
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator>
    void MapDecoder<algorithm, T>::branchUpdate(Codec::iterator<InputIterator> input)
    {
      auto parity = input.at(Codec::Parity);
      auto syst = input.at(Codec::Syst);
      auto branch = branch_.begin();
      for (size_t i = 0; i < structure().length() + structure().tailLength(); ++i) {
        for (BitField<size_t> j = 0; j < structure().trellis().outputCount(); ++j) {
          buffer_[j] = mergeMetrics(parity, structure().parityWidth(), structure().trellis().outputWidth(), j);
        }
        auto branchTmp = branch;
        for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
          for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
            branch[k] = buffer_[size_t(output[k])];
          }
          output += structure().trellis().inputCount();
          branch += structure().trellis().inputCount();
        }
        
        if (input.count(Codec::Syst)) {
          branch = branchTmp;
          for (BitField<size_t> j = 0; j < structure().trellis().inputCount(); ++j) {
            buffer_[j] = mergeMetrics(syst, structure().systWidth(), structure().trellis().inputWidth(), j);
          }
          for (size_t j = 0; j < structure().trellis().stateCount(); ++j) {
            for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
              branch[k] += buffer_[k];
            }
            branch += structure().trellis().inputCount();
          }
        }
        parity += structure().outputLength();
        syst += structure().inputLength();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    void MapDecoder<algorithm, T>::forwardUpdate()
    {
      auto forward = forward_.begin();
      auto branch = branch_.cbegin();
      
      *forward = 0;
      std::fill(forward+1, forward + structure().trellis().stateCount(), -std::numeric_limits<T>::infinity());
      
      for (; forward < forward_.end() - structure().trellis().stateCount();) {
        forwardUpdateImpl(forward, branch);
        forward += structure().trellis().stateCount();
        branch += structure().trellis().tableSize();
        T max = -std::numeric_limits<T>::infinity();
        for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
          forward[j] = forward[j];
          max = std::max(forward[j], max);
        }
        for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
          forward[j] -= max;
        }
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    void MapDecoder<algorithm, T>::backwardUpdate()
    {
      auto backward = backward_.end()-structure().trellis().stateCount();
      switch (structure().termination()) {
        case Trellis::Tail:
          *backward = 0;
          std::fill(backward+1, backward + structure().trellis().stateCount(), -std::numeric_limits<T>::infinity());
          break;
          
        default:
        case Trellis::Truncate:
          std::fill(backward, backward + structure().trellis().stateCount(), T(0));
          break;
      }
      backward -= structure().trellis().stateCount();
      auto branch = branch_.cend()-structure().trellis().tableSize();
      
      for ( ; backward >= backward_.begin();) {
        backwardUpdateImpl(backward, branch);
        T max = -std::numeric_limits<T>::infinity();
        for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
          max = std::max(backward[j], max);
        }
        for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
          backward[j] -= max;
        }
        backward -= structure().trellis().stateCount();
        branch -= structure().trellis().tableSize();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator, class OutputIterator>
    void MapDecoder<algorithm, T>::aPosterioriUpdate(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output)
    {
      auto systOut = output.at(Codec::Syst);
      auto systIn = input.at(Codec::Syst);
      auto parityOut = output.at(Codec::Parity);
      auto parityIn = input.at(Codec::Parity);
      auto msgOut = output.at(Codec::Msg);
      
      for (size_t i = 0; i < structure().length() + structure().tailLength(); ++i) {
        auto branch = branch_.begin() + i * structure().trellis().tableSize();
        auto forward = forward_.cbegin() + i * structure().trellis().stateCount();
        auto backward = backward_.cbegin() + i * structure().trellis().stateCount();
        
        for (auto state = structure().trellis().beginState(); state < structure().trellis().endState(); ) {
          for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
            branch[input] = branch[input] + *forward + backward[size_t(state[input])];
          }
          state += structure().trellis().inputCount();
          branch += structure().trellis().inputCount();
          ++forward;
        }
        
        if (output.count(Codec::Syst) || (output.count(Codec::Msg) && i < structure().length())) {
          for (size_t j = 0; j < structure().trellis().inputWidth(); j += structure().msgWidth()) {
            branch = branch_.begin() + i * structure().trellis().tableSize();
            msgUpdateImpl(branch, j);
            
            if (output.count(Codec::Syst)) {
              if (input.count(Codec::Syst)) {
                for (size_t k = 1; k < structure().systCount(); ++k) {
                  systOut[k-1] = structure().scalingFactor() * (buffer_[k] - buffer_[0] - systIn[k-1]);
                }
              }
              else {
                for (size_t k = 1; k < structure().systCount(); ++k) {
                  systOut[k-1] = structure().scalingFactor() * (buffer_[k] - buffer_[0]);
                }
              }
            }
            if (output.count(Codec::Msg) && i < structure().length()) {
              for (size_t k = 1; k < structure().msgCount(); ++k) {
                msgOut[k-1] = structure().scalingFactor() * (buffer_[k] - buffer_[0]);
              }
            }
            systIn += structure().systCount()-1;
            systOut += structure().systCount()-1;
            msgOut += structure().msgCount()-1;
          }
        }
        if (output.count(Codec::Parity)) {
          for (size_t j = 0; j < structure().trellis().outputWidth(); j += structure().parityWidth()) {
            branch = branch_.begin() + i * structure().trellis().tableSize();
            parityUpdateImpl(branch, j);
            
            if (input.count(Codec::Parity)) {
              for (size_t k = 1; k < structure().parityCount(); ++k) {
                parityOut[k-1] = structure().scalingFactor() * (buffer_[k] - buffer_[0] - parityIn[k-1]);
              }
            }
            else {
              for (size_t k = 1; k < structure().parityCount(); ++k) {
                parityOut[k-1] = structure().scalingFactor() * (buffer_[k] - buffer_[0]);
              }
            }
            parityIn += structure().parityCount()-1;
            parityOut += structure().parityCount()-1;
          }
        }
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<U::value>::type*>
    void MapDecoder<algorithm, T>::forwardUpdateImpl(typename std::vector<T>::iterator forward, typename std::vector<T>::const_iterator branch)
    {
      std::fill(forward + structure().trellis().stateCount(), forward + 2*structure().trellis().stateCount(), -std::numeric_limits<T>::infinity());
      auto state = structure().trellis().beginState();
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          auto & forwardMetricRef = forward[structure().trellis().stateCount() + size_t(state[k])];
          forwardMetricRef =
          logSum_(forwardMetricRef, forward[j] + branch[k]);
        }
        branch += structure().trellis().inputCount();
        state += structure().trellis().inputCount();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<!U::value>::type*>
    void MapDecoder<algorithm, T>::forwardUpdateImpl(typename std::vector<T>::iterator forward, typename std::vector<T>::const_iterator branch)
    {
      auto state = structure().trellis().beginState();
      auto buffer = buffer_.begin();
      auto maxMetric = buffer + structure().trellis().stateCount()*structure().trellis().inputCount();
      std::fill(maxMetric, maxMetric + structure().trellis().stateCount(), -std::numeric_limits<T>::infinity());
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          buffer[k] = forward[j] + branch[k];
          maxMetric[size_t(state[k])] = std::max(buffer[k], maxMetric[size_t(state[k])]);
        }
        branch += structure().trellis().inputCount();
        buffer += structure().trellis().inputCount();
        state += structure().trellis().inputCount();
      }
      forward += structure().trellis().stateCount();
      std::fill(forward, forward + structure().trellis().stateCount(), 0);
      state = structure().trellis().beginState();
      buffer = buffer_.begin();
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          buffer[k] = logSum_.prior(buffer[k], maxMetric[size_t(state[k])]);
          forward[size_t(state[k])] = logSum_(buffer[k], forward[size_t(state[k])]);
        }
        buffer += structure().trellis().inputCount();
        state += structure().trellis().inputCount();
      }
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        forward[j] = logSum_.post(forward[j], maxMetric[j]);
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<U::value>::type*>
    void MapDecoder<algorithm, T>::backwardUpdateImpl(typename std::vector<T>::iterator backward, typename std::vector<T>::const_iterator branch)
    {
      std::fill(backward, backward + structure().trellis().stateCount(), -std::numeric_limits<T>::infinity());
      auto state = structure().trellis().beginState();
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          backward[j] =
          logSum_(backward[j], backward[size_t(state[k])+structure().trellis().stateCount()] + branch[k]);
        }
        branch += structure().trellis().inputCount();
        state += structure().trellis().inputCount();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<!U::value>::type*>
    void MapDecoder<algorithm, T>::backwardUpdateImpl(typename std::vector<T>::iterator backward, typename std::vector<T>::const_iterator branch)
    {
      auto state = structure().trellis().beginState();
      auto buffer = buffer_.begin();
      std::fill(backward, backward + structure().trellis().stateCount(), 0);
      for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
        T max = -std::numeric_limits<T>::infinity();
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          buffer[k] = backward[size_t(state[k])+structure().trellis().stateCount()] + branch[k];
          max = std::max(buffer[k], max);
        }
        for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
          buffer[k] = logSum_.prior(buffer[k], max);
          backward[j] = logSum_(buffer[k], backward[j]);
        }
        backward[j] = logSum_.post(backward[j], max);
        branch += structure().trellis().inputCount();
        buffer += structure().trellis().inputCount();
        state += structure().trellis().inputCount();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<U::value>::type*>
    void MapDecoder<algorithm, T>::msgUpdateImpl(typename std::vector<T>::iterator branch, size_t j)
    {
      auto metric = buffer_.begin();
      std::fill(metric, metric + structure().systCount(), -std::numeric_limits<T>::infinity());
      for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          metric[input.test(j, structure().systWidth())] = logSum_(metric[input.test(j, structure().systWidth())], branch[input]);
        }
        branch += structure().trellis().inputCount();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<U::value>::type*>
    void MapDecoder<algorithm, T>::parityUpdateImpl(typename std::vector<T>::iterator branch, size_t j)
    {
      auto metric = buffer_.begin();
      std::fill(metric, metric + structure().parityCount(), -std::numeric_limits<T>::infinity());
      for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          metric[output[input].test(j, structure().parityWidth())] = logSum_(metric[output[input].test(j, structure().parityWidth())], branch[input]);
        }
        branch += structure().trellis().inputCount();
        output += structure().trellis().inputCount();
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<!U::value>::type*>
    void MapDecoder<algorithm, T>::msgUpdateImpl(typename std::vector<T>::iterator branch, size_t j)
    {
      auto metric = buffer_.begin();
      auto max = metric + structure().systCount();
      std::fill(max, max + structure().systCount(), -std::numeric_limits<T>::infinity());
      std::fill(metric, metric + structure().systCount(), T(0));
      auto branchTmp = branch;
      for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          max[input.test(j, structure().systWidth())] = std::max(branch[input], max[input.test(j, structure().systWidth())]);
        }
        branch += structure().trellis().inputCount();
      }
      branch = branchTmp;
      for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          metric[input.test(j, structure().systWidth())] = logSum_(logSum_.prior(branch[input], max[input.test(j, structure().systWidth())]), metric[input.test(j, structure().systWidth())]);
        }
        branch += structure().trellis().inputCount();
      }
      for (size_t i = 0; i < structure().systCount(); ++i) {
        metric[i] = logSum_.post(metric[i], max[i]);
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class U, typename std::enable_if<!U::value>::type*>
    void MapDecoder<algorithm, T>::parityUpdateImpl(typename std::vector<T>::iterator branch, size_t j)
    {
      auto metric = buffer_.begin();
      auto max = metric + structure().parityCount();
      std::fill(max, max + structure().parityCount(), -std::numeric_limits<T>::infinity());
      std::fill(metric, metric + structure().parityCount(), T(0));
      auto branchTmp = branch;
      for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          max[output[input].test(j, structure().parityWidth())] = std::max(branch[input], max[output[input].test(j, structure().parityWidth())]);
        }
        branch += structure().trellis().inputCount();
        output += structure().trellis().inputCount();
      }
      branch = branchTmp;
      for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
        for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
          metric[output[input].test(j, structure().parityWidth())] = logSum_(logSum_.prior(branch[input], max[output[input].test(j, structure().systWidth())]), metric[output[input].test(j, structure().parityWidth())]);
        }
        branch += structure().trellis().inputCount();
        output += structure().trellis().inputCount();
      }
      for (size_t i = 0; i < structure().parityCount(); ++i) {
        metric[i] = logSum_.post(metric[i], max[i]);
      }
    }
    
  }
  
}

#endif
