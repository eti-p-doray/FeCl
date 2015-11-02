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

#include "../../Turbo.h"
#include "../../Convolutional/MapDecoder/MapDecoder.h"

namespace fec {

  /**
   *  This class contains the implementation of iterative decoder.
   *  This algorithm is used for decoding in a TurboCodec.
   */
  class TurboDecoder
  {
  public:
    static std::unique_ptr<TurboDecoder> create(const Turbo::detail::Structure&);
    virtual ~TurboDecoder() = default;
    
    inline const Turbo::detail::Structure& structure() const {return structure_;}
    
    void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n);
    void soDecodeBlocks(Codec::detail::InputIterator input, Codec::detail::OutputIterator output, size_t n);
    
  protected:
    TurboDecoder(const Turbo::detail::Structure& codeStructure);
    TurboDecoder() = default;
    
    virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg) = 0;
    virtual void soDecodeBlock(Codec::detail::InputIterator input, Codec::detail::OutputIterator output) = 0;
    
    std::vector<std::unique_ptr<MapDecoder>> code_;
    
    std::vector<LlrType> extrinsic_;
    std::vector<LlrType> extrinsicBuffer_;
    std::vector<LlrType> parityIn_;
    std::vector<LlrType> parityOut_;
    
  private:
    Turbo::detail::Structure structure_;
  };
  
}

#endif
