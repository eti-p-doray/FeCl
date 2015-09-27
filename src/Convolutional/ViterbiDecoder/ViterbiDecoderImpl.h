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
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_VITERBI_DECODER_IMPL_H
#define FEC_VITERBI_DECODER_IMPL_H

#include <vector>
#include <memory>

#include "ViterbiDecoder.h"

namespace fec {

/**
 *  This class contains the implementation of the viterbi decoder.
 *  This algorithm is used for simple decoding in a ConvolutionalCodec.
 */
  template <class LlrMetrics>
  class ViterbiDecoderImpl : public ViterbiDecoder
  {
  public:
    ViterbiDecoderImpl(const Convolutional::Structure&);
    ~ViterbiDecoderImpl() = default;
    
    virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg);
    
  protected:
    std::vector<typename LlrMetrics::Type> previousPathMetrics_;
    std::vector<typename LlrMetrics::Type> nextPathMetrics_;
    std::vector<typename LlrMetrics::Type> branchMetrics_;
    std::vector<BitField<uint16_t>> stateTraceBack_;
    std::vector<BitField<uint16_t>> inputTraceBack_;

  private:    
    FloatLlrMetrics llrMetrics_;
  };
  
}

#endif
