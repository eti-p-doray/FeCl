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

#ifndef FEC_BP_DECODER_IMPL_H
#define FEC_BP_DECODER_IMPL_H

#include <algorithm>
#include <cmath>

#include "BpDecoder.h"
#include "../LlrMetrics.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the concrete implementation of the belief propagation decoder.
     *  This algorithm is used for decoding in an LdpcCodec.
     *  The reason for this class is to offer an common interface of bp decoders
     *  while allowing the compiler to inline implementation specific functions
     *  by using templates instead of polymorphism.
     */
    template <class LlrMetrics, template <class> class BoxSumAlg>
    class BpDecoderImpl : public BpDecoder {
    public:
      BpDecoderImpl(const Ldpc::Structure& structure);
      ~BpDecoderImpl() = default;
      
      void decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg) override;
      void soDecodeBlock(detail::Codec::const_iterator<double> first, detail::Codec::iterator<double> output) override;
      
    private:
      void checkUpdate(size_t i);
      void bitUpdate();
      
      std::vector<BitField<size_t>> hardParity_;
      
      std::vector<typename LlrMetrics::Type> parity_;
      std::vector<typename LlrMetrics::Type> bitMetrics_;
      std::vector<typename LlrMetrics::Type> checkMetrics_;
      std::vector<typename LlrMetrics::Type> checkMetricsBuffer_;
      
      LlrMetrics llrMetrics_;
      BoxSumAlg<LlrMetrics> boxSum_;
    };
    
  }
  
}

#endif
