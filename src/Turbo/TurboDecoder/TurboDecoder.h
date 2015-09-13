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
 
 Declaration of TurboCodecImpl class
 ******************************************************************************/

#ifndef TURBO_DECODER_H
#define TURBO_DECODER_H

#include "../Turbo.h"
#include "../../Convolutional/MapDecoder/MapDecoder.h"

namespace fec {

  /**
   *  This class contains the implementation of iterative decoder.
   *  This algorithm is used for decoding in a TurboCodec.
   */
  class TurboDecoder
  {
  public:
    static std::unique_ptr<TurboDecoder> create(const Turbo::Structure&);
    virtual ~TurboDecoder() = default;
    
    inline const Turbo::Structure& structure() const {return structure_;}
    
    void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n);
    void soDecodeBlocks(Codec::InputIterator input, Codec::OutputIterator output, size_t n);
    
  protected:
    TurboDecoder(const Turbo::Structure& codeStructure);
    TurboDecoder() = default;
    
    virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg) = 0;
    virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output) = 0;
    
    std::vector<std::unique_ptr<MapDecoder>> code_;
    
    std::vector<LlrType> extrinsic_;
    std::vector<LlrType> extrinsicBuffer_;
    std::vector<LlrType> parityIn_;
    std::vector<LlrType> systOut_;
    
  private:
    Turbo::Structure structure_;
  };
  
}

#endif
