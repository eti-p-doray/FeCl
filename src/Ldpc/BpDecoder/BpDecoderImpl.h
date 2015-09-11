/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of BpDecoderImpl class
 ******************************************************************************/

#ifndef BP_DECODER_IMPL_H
#define BP_DECODER_IMPL_H

#include <algorithm>
#include <cmath>

#include "BpDecoder.h"

namespace fec {

  /**
   *  This class contains the concrete implementation of the belief propagation decoder.
   *  This algorithm is used for decoding in an LdpcCode.
   *  The reason for this class is to offer an common interface of bp decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
  template <class LlrMetrics, template <class> class BoxSumAlg>
  class BpDecoderImpl : public BpDecoder {
  public:
    BpDecoderImpl(const Ldpc::Structure& structure);
    ~BpDecoderImpl() = default;
    
  protected:
    virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg);
    virtual void soDecodeBlock(Code::InputIterator input, Code::
                               OutputIterator output);
    
  private:
    void checkUpdate();
    void bitUpdate();
    
    std::vector<uint8_t> hardParity_;
    
    std::vector<typename LlrMetrics::Type> parity_;
    std::vector<typename LlrMetrics::Type> bitMetrics_;
    std::vector<typename LlrMetrics::Type> checkMetrics_;
    std::vector<typename LlrMetrics::Type> checkMetricsBuffer_;
    
    LlrMetrics llrMetrics_;
    BoxSumAlg<LlrMetrics> boxSum_;
  };
  
}

#endif
