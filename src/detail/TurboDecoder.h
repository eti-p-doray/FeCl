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

#ifndef FEC_TURBO_DECODER_H
#define FEC_TURBO_DECODER_H

#include "Turbo.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the implementation of iterative decoder.
     *  This algorithm is used for decoding in a TurboCodec.
     */
    template <class T, template <class> class LogSumAlg>
    class TurboDecoder
    {
    public:
      TurboDecoder(const Turbo::Structure& structure);
      TurboDecoder(const TurboDecoder&) = default;
      virtual ~TurboDecoder() = default;
      
      template <class InputIterator, class OutputIterator>
      void decode(InputIterator parity, OutputIterator msg);
      
      template <class InputIterator, class OutputIterator>
      void soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output);
    
    private:
      inline const Turbo::Structure& structure() const {return structure_;}
      
      void aPosterioriUpdate();
      
      void customActivationUpdate(size_t i, size_t stage, bool outputParity);
      
      void serialTransferUpdate(size_t i);
      void parallelTransferUpdate();
      void customTransferUpdate(size_t stage, size_t i);
      
      std::vector<MapDecoder<T,LogSumAlg>> code_;
      
      std::vector<T> state_;
      std::vector<T> stateBuffer_;
      std::vector<T> parityIn_;
      std::vector<T> parityOut_;
      
      Turbo::Structure structure_;
    };
    
    template <class T, template <class> class LogSumAlg>
    TurboDecoder<T, LogSumAlg>::TurboDecoder(const Turbo::Structure& structure) : structure_(structure)
    {
      for (size_t i = 0; i < this->structure().constituentCount(); ++i) {
        code_.push_back(MapDecoder<T,LogSumAlg>(this->structure().constituent(i)));
      }
      state_.resize(this->structure().stateSize());
      stateBuffer_.resize(this->structure().stateSize());;
      parityIn_.resize(this->structure().paritySize());
      parityOut_.resize(this->structure().paritySize());
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class InputIterator, class OutputIterator>
    void TurboDecoder<T,LogSumAlg>::decode(InputIterator parity, OutputIterator msg)
    {
      std::copy(parity, parity + structure().paritySize(), parityIn_.begin());
      std::fill(state_.begin(), state_.end(), 0);
      for (size_t i = 0; i < structure().iterations(); ++i) {
        if (structure().schedulingType() == Parallel) {
          parallelTransferUpdate();
        }
        
        if (structure().schedulingType() == Custom) {
          for (size_t j = 0; j < structure().scheduling().size(); ++j) {
            stateBuffer_ = state_;
            for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
              customTransferUpdate(j, k);
            }
            std::swap(stateBuffer_, state_);
            customActivationUpdate(i, j, false);
          }
        } else {
          auto parityIn = parityIn_.begin() + structure().systSize();
          auto state = state_.begin();
          for (size_t j = 0; j < structure().constituentCount(); ++j) {
            if (structure().schedulingType() == Serial) {
              serialTransferUpdate(j);
            }
            code_[j].setScalingFactor(structure().scalingFactor(i, j));
            auto inputTmp = Codec::iterator<typename std::vector<T>::const_iterator>{{Codec::Parity, parityIn, 1}, {Codec::Syst, state, 1}};
            auto outputTmp = Codec::iterator<typename std::vector<T>::iterator>{{Codec::Syst, state, 1}};
            code_[j].soDecode(inputTmp, outputTmp);
            
            state += structure().constituent(j).systSize();
            parityIn += structure().constituent(j).paritySize();
          }
        }
      }
      std::copy(parityIn_.begin(), parityIn_.begin()+structure().msgSize(), parityOut_.begin());
      aPosterioriUpdate();
      
      for (size_t i = 0; i < structure().msgSize(); ++i) {
        msg[i] = parityOut_[i] > 0;
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    template <class InputIterator, class OutputIterator>
    void TurboDecoder<T,LogSumAlg>::soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output)
    {
      std::copy(input.at(Codec::Parity), input.at(Codec::Parity) + structure().paritySize(), parityIn_.begin());
      if (input.count(Codec::Syst)) {
        auto syst = input.at(Codec::Syst);
        for (size_t i = 0; i < structure().systSize(); ++i) {
          parityIn_[i] += syst[i];
        }
      }
      
      if (input.count(Codec::State)) {
        std::copy(input.at(Codec::State), input.at(Codec::State)+structure().stateSize(), state_.begin());
      }
      else {
        std::fill(state_.begin(), state_.end(), 0);
      }
      
      if (structure().iterations() == 0) {
        if (output.count(Codec::Parity)) {
          std::fill(output.at(Codec::Parity)+structure().systSize(), output.at(Codec::Parity)+structure().paritySize(), 0);
        }
      }
      
      for (size_t i = 0; i < structure().iterations(); ++i) {
        if (structure().schedulingType() == Parallel) {
          parallelTransferUpdate();
        }
        
        if (structure().schedulingType() == Custom) {
          for (size_t j = 0; j < structure().scheduling().size(); ++j) {
            stateBuffer_ = state_;
            for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
              customTransferUpdate(j, k);
            }
            std::swap(stateBuffer_, state_);
            customActivationUpdate(i, j, output.count(Codec::Parity));
          }
        } else {
          auto parityIn = parityIn_.begin() + structure().systSize();
          auto parityOut = parityOut_.begin() + structure().systSize();
          auto state = state_.begin();
          for (size_t j = 0; j < structure().constituentCount(); ++j) {
            if (structure().schedulingType() == Serial) {
              serialTransferUpdate(j);
            }
            
            auto inputTmp = Codec::iterator<typename std::vector<T>::const_iterator>{{Codec::Parity, parityIn, 1}, {Codec::Syst, state, 1}};
            auto outputTmp = Codec::iterator<typename std::vector<T>::iterator>{{Codec::Syst, state, 1}};
            if (i == structure().iterations()-1 && output.count(Codec::Parity)) {
              outputTmp.insert(Codec::Parity, parityOut, 1);
            }
            code_[j].soDecode(inputTmp, outputTmp);
            
            state += structure().constituent(j).systSize();
            parityIn += structure().constituent(j).paritySize();
            parityOut += structure().constituent(j).paritySize();
          }
        }
      }
      std::fill(parityOut_.begin(), parityOut_.begin() + structure().systSize(), 0);
      aPosterioriUpdate();
      
      if (output.count(Codec::Syst)) {
        std::copy(parityOut_.begin(), parityOut_.begin()+structure().systSize(), output.at(Codec::Syst));
      }
      if (output.count(Codec::Parity)) {
        std::copy(parityOut_.begin(), parityOut_.end(), output.at(Codec::Parity));
      }
      if (output.count(Codec::State)) {
        std::copy(state_.begin(), state_.end(), output.at(Codec::State));
      }
      if (output.count(Codec::Msg)) {
        auto msg = output.at(Codec::Msg);
        for (size_t i = 0; i < structure().msgSize(); ++i) {
          msg[i] = parityIn_[i] + parityOut_[i];
        }
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    void TurboDecoder<T,LogSumAlg>::aPosterioriUpdate()
    {
      auto state = state_.begin();
      auto tail = parityOut_.begin() + structure().msgSize();
      auto syst = parityOut_.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
          syst[structure().interleaver(j)[k]] += state[k];
        }
        state += structure().constituent(j).msgSize();
        for (size_t k = 0; k < structure().constituent(j).tailLength(); ++k) {
          tail[k] += state[k];
        }
        state += structure().constituent(j).tailSize();
        tail += structure().constituent(j).tailSize();
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    void TurboDecoder<T,LogSumAlg>::customActivationUpdate(size_t i, size_t stage, bool outputParity)
    {
      auto parityIn = parityIn_.begin() + structure().systSize();
      auto parityOut = parityOut_.begin() + structure().systSize();
      auto state = state_.begin();
      auto activation = structure().scheduling()[stage].activation.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        while (activation != structure().scheduling()[stage].activation.end() && *activation < j) {++activation;}
        if (activation != structure().scheduling()[stage].activation.end() && *activation == j) {
          code_[j].setScalingFactor(structure().scalingFactor(i, j));
          
          auto inputTmp = Codec::iterator<typename std::vector<T>::const_iterator>{{Codec::Parity, parityIn, 1}, {Codec::Syst, state, 1}};
          auto outputTmp = Codec::iterator<typename std::vector<T>::iterator>{{Codec::Syst, state, 1}};
          if (i == structure().iterations()-1 && outputParity) {
            outputTmp.insert(Codec::Parity, parityOut, 1);
          }
          
          code_[j].soDecode(inputTmp, outputTmp);
        } else if (activation == structure().scheduling()[stage].activation.end()) {
          break;
        }
        
        state += structure().constituent(j).systSize();
        parityIn += structure().constituent(j).paritySize();
        parityOut += structure().constituent(j).paritySize();
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    void TurboDecoder<T,LogSumAlg>::parallelTransferUpdate()
    {
      auto state = state_.begin();
      auto stateTmp = stateBuffer_.begin();
      
      auto systTail = parityIn_.begin() + structure().msgSize();
      auto syst = parityOut_.begin();
      std::fill(parityOut_.begin(), parityOut_.begin() + structure().msgSize(), 0);
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
          stateTmp[k] = state[k];
          state[k] = syst[structure().interleaver(j)[k]];
          syst[structure().interleaver(j)[k]] += stateTmp[k];
        }
        state += structure().constituent(j).msgSize();
        stateTmp += structure().constituent(j).msgSize();
        
        std::copy(systTail, systTail + structure().constituent(j).tailSize(), state);
        
        state += structure().constituent(j).tailSize();
        systTail += structure().constituent(j).tailSize();
      }
      std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
      
      for (int64_t j = structure().constituentCount()-1; j >= 0; --j) {
        state -= structure().constituent(j).systSize();
        stateTmp -= structure().constituent(j).msgSize();
        for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
          state[k] += syst[structure().interleaver(j)[k]];
          syst[structure().interleaver(j)[k]] += stateTmp[k];
        }
      }
    }
    
    template <class T, template <class> class LogSumAlg>
    void TurboDecoder<T,LogSumAlg>::serialTransferUpdate(size_t i)
    {
      auto state = state_.begin();
      auto systTail = parityIn_.begin() + structure().msgSize();
      auto syst = parityOut_.begin();
      std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
      for (size_t j = 0; j < i; ++j) {
        for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
          syst[structure().interleaver(j)[k]] += state[k];
        }
        state += structure().constituent(j).systSize();
        systTail += structure().constituent(j).tailSize();
      }
      auto stateTmp = state;
      state += structure().constituent(i).msgSize();
      std::copy(systTail, systTail + structure().constituent(i).tailSize(), state);
      state += structure().constituent(i).tailSize();
      for (size_t j = i+1; j < structure().constituentCount(); ++j) {
        for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
          syst[structure().interleaver(j)[k]] += state[k];
        }
        state += structure().constituent(j).systSize();
      }
      
      structure().interleaver(i).permuteBlock(syst, stateTmp);
    }
    
    template <class T, template <class> class LogSumAlg>
    void TurboDecoder<T,LogSumAlg>::customTransferUpdate(size_t stage, size_t src)
    {
      size_t i = structure().scheduling()[stage].activation[src];
      auto state = state_.begin();
      auto stateTmp = stateBuffer_.begin();
      auto systTail = parityIn_.begin() + structure().msgSize();
      auto syst = parityOut_.begin();
      auto transfer = structure().scheduling()[stage].transfer[src].begin();
      std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
      for (size_t j = 0; j < i; ++j) {
        while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
        if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
          for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
            syst[structure().interleaver(j)[k]] += state[k];
          }
        }
        state += structure().constituent(j).systSize();
        stateTmp += structure().constituent(j).systSize();
        systTail += structure().constituent(j).tailSize();
      }
      auto extrinsicConst = stateTmp;
      state += structure().constituent(i).msgSize();
      std::copy(systTail, systTail + structure().constituent(i).tailSize(), state);
      state += structure().constituent(i).tailSize();
      for (size_t j = i+1; j < structure().constituentCount(); ++j) {
        while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
        if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
          for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
            syst[structure().interleaver(j)[k]] += state[k];
          }
        } else if (transfer == structure().scheduling()[stage].transfer[src].end()) {
          break;
        }
        state += structure().constituent(j).systSize();
      }
      
      structure().interleaver(i).permuteBlock(syst, extrinsicConst);
    }

    
  }
  
}

#endif
