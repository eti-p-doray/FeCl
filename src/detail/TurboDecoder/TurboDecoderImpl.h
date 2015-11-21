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

#ifndef FEC_TURBO_DECODER_IMPL_H
#define FEC_TURBO_DECODER_IMPL_H

#include "TurboDecoder.h"

namespace fec {
  
  namespace detail {
    
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
      
      virtual void decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg);
      virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output);
      
    private:
      void aPosterioriUpdate();
      
      void customActivationUpdate(size_t i, size_t stage);
      
      void serialTransferUpdate(size_t i);
      void parallelTransferUpdate();
      void customTransferUpdate(size_t stage, size_t i);
    };
    
  }
  
}

#endif
