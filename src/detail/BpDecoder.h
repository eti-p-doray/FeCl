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

#ifndef FEC_BP_DECODER_H
#define FEC_BP_DECODER_H

#include <algorithm>
#include <cmath>

#include "Ldpc.h"
#include "LlrMetrics.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the concrete implementation of the belief propagation decoder.
     *  This algorithm is used for decoding in an LdpcCodec.
     *  The reason for this class is to offer an common interface of bp decoders
     *  while allowing the compiler to inline implementation specific functions
     *  by using templates instead of polymorphism.
     */
    template <DecoderAlgorithm algorithm, class T>
    class BpDecoder {
    public:
      BpDecoder(const Ldpc::Structure& structure);
      BpDecoder(const BpDecoder&) = default;
      ~BpDecoder() = default;
      
      template <class InputIterator, class OutputIterator>
      void decode(InputIterator parity, OutputIterator msg);
      
      template <class InputIterator, class OutputIterator>
      void soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output);
      
    private:
      inline const Ldpc::Structure& structure() const {return structure_;} /**< Access the code structure */
      
      void checkUpdate(size_t i);
      void bitUpdate();
      
      std::vector<BitField<size_t>> hardParity_;
      
      std::vector<T> parity_;
      std::vector<T> bit_;
      std::vector<T> check_;
      std::vector<T> buffer_;
      
      BoxSum<algorithm,T> boxSum_;
      
      Ldpc::Structure structure_;
    };
    
    template <DecoderAlgorithm algorithm, class T>
    BpDecoder<algorithm, T>::BpDecoder(const Ldpc::Structure& structure) : structure_(structure)
    {
      hardParity_.resize(this->structure().checks().cols());
      parity_.resize(this->structure().checks().cols());
      check_.resize(this->structure().checks().size());
      buffer_.resize(this->structure().checks().size());
      bit_.resize(this->structure().checks().cols());
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator, class OutputIterator>
    void BpDecoder<algorithm, T>::decode(InputIterator parity, OutputIterator msg)
    {
      std::copy(parity, parity+structure().checks().cols(), parity_.begin());
      
      if (structure().iterations() > 0) {
        for (size_t i = 0; i < structure().checks().size(); ++i) {
          check_[i] = parity_[structure().checks().at(i)];
        }
      }
      
      bool success = false;
      for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
        checkUpdate(i);
        bitUpdate();
        
        for (size_t j = 0; j < structure().checks().cols(); ++j) {
          hardParity_[j] = (bit_[j] >= 0.0);
        }
        if (structure().check(hardParity_.begin())) {
          success = true;
          break;
        }
      }
      checkUpdate(structure().iterations()-1);
      
      std::copy(parity_.begin(), parity_.end(), bit_.begin());
      for (size_t i = 0; i < structure().checks().size(); ++i) {
        bit_[structure().checks().at(i)] += check_[i];
      }
      for (size_t i = 0; i < structure().msgSize(); ++i) {
        msg[i] = bit_[i] >= 0;
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    template <class InputIterator, class OutputIterator>
    void BpDecoder<algorithm, T>::soDecode(Codec::iterator<InputIterator> input, Codec::iterator<OutputIterator> output)
    {
      std::copy(input.at(Codec::Parity), input.at(Codec::Parity)+structure().checks().cols(), parity_.begin());
      if (input.count(Codec::Syst)) {
        auto syst = input.at(Codec::Syst);
        for (size_t i = 0; i < structure().systSize(); ++i) {
          parity_[i] += syst[i];
        }
      }
      if (input.count(Codec::State)) {
        std::copy(input.at(Codec::State), input.at(Codec::State)+structure().stateSize(), check_.begin());
      }
      
      if (structure().iterations() > 0) {
        if (input.count(Codec::State)) {
          bitUpdate();
        }
        else {
          for (size_t i = 0; i < structure().checks().size(); ++i) {
            check_[i] = parity_[structure().checks().at(i)];
          }
        }
      }
      
      bool success = false;
      for (int64_t i = 0; i < structure().iterations() - 1; ++i) {
        checkUpdate(i);
        bitUpdate();
        
        for (size_t j = 0; j < structure().checks().cols(); ++j) {
          hardParity_[j] = (bit_[j] >= 0.0);
        }
        if (structure().check(hardParity_.begin())) {
          success = true;
          break;
        }
      }
      checkUpdate(structure().iterations()-1);
      
      std::fill(bit_.begin(), bit_.end(), 0);
      for (size_t i = 0; i < structure().checks().size(); ++i) {
        bit_[structure().checks().at(i)] += check_[i];
      }
      
      if (output.count(Codec::Syst)) {
        std::copy(bit_.begin(), bit_.begin()+structure().systSize(), output.at(Codec::Syst));
      }
      if (output.count(Codec::Parity)) {
        std::copy(bit_.begin(), bit_.begin()+structure().checks().cols(), output.at(Codec::Parity));
      }
      if (output.count(Codec::State)) {
        std::copy(check_.begin(), check_.end(), output.at(Codec::State));
      }
      if (output.count(Codec::Msg)) {
        auto msg = output.at(Codec::Msg);
        for (size_t i = 0; i < structure().msgSize(); ++i) {
          msg[i] = parity_[i] + bit_[i];
        }
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    void BpDecoder<algorithm, T>::checkUpdate(size_t i)
    {
      auto checkMetric = check_.begin();
      auto checkMetricTmp = buffer_.begin();
      for (auto check = structure().checks().begin(); check < structure().checks().end();  ++check) {
        auto first = checkMetric;
        size_t size = check->size();
        double sf = structure().scalingFactor(i, size);
        
        double prod = boxSum_.prior(*first);
        for (size_t j = 1; j < size-1; ++j) {
          checkMetricTmp[j] = boxSum_.prior(first[j]);
          first[j] = prod;
          prod = boxSum_(prod, checkMetricTmp[j]);
        }
        checkMetricTmp[size-1] = boxSum_.prior(first[size-1]);
        first[size-1] = sf *  (boxSum_.post(prod));
        prod = checkMetricTmp[size-1];
        for (size_t j = size-2; j > 0; --j) {
          first[j] = sf *  (boxSum_.post( boxSum_(first[j], prod) ));
          prod = boxSum_(prod, checkMetricTmp[j]);
        }
        *first = sf *  (boxSum_.post(prod));
        
        checkMetric += size;
      }
    }
    
    template <DecoderAlgorithm algorithm, class T>
    void BpDecoder<algorithm, T>::bitUpdate()
    {
      std::fill(bit_.begin(), bit_.end(), 0);
      for (size_t i = 0; i < structure().checks().size(); ++i) {
        buffer_[i] = check_[i];
        check_[i] = bit_[structure().checks().at(i)];
        bit_[structure().checks().at(i)] += buffer_[i];
      }
      
      std::copy(parity_.begin(), parity_.begin() + bit_.size(), bit_.begin());
      
      for (int64_t i = structure().checks().size() - 1; i >= 0; --i) {
        check_[i] += bit_[structure().checks().at(i)];
        bit_[structure().checks().at(i)] += buffer_[i];
      }
    }
    
  }
  
}

#endif
