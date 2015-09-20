/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
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
 
 Declaration of TurboDecoderImpl class
 ******************************************************************************/

#ifndef FEC_TURBO_DECODER_IMPL_H
#define FEC_TURBO_DECODER_IMPL_H

#include "TurboDecoder.h"

namespace fec {
  
  /**
   *  This class contains the implementation of iterative decoder.
   *  This algorithm is used for decoding in a TurboCodec.
   */
  
  class TurboDecoderImpl : public TurboDecoder
  {
  public:
    TurboDecoderImpl(const Turbo::Structure& structure);
    virtual ~TurboDecoderImpl() = default;
    
  protected:
    TurboDecoderImpl() = default;
    
    virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg);
    virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output);
    
  private:
    void aPosterioriUpdate();
    void serialSharingUpdate(size_t i);
    void parallelSharingUpdate();
  };
  
}

#endif
