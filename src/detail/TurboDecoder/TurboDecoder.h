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

#include "../Turbo.h"
#include "../MapDecoder/MapDecoder.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the implementation of iterative decoder.
     *  This algorithm is used for decoding in a TurboCodec.
     */
    class TurboDecoder
    {
    public:
      static std::unique_ptr<TurboDecoder> create(const Turbo::Structure&);
      virtual ~TurboDecoder() = default;
      
      void decodeBlocks(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n);
      void soDecodeBlocks(Codec::InputIterator input, Codec::OutputIterator output, size_t n);
      
    protected:
      TurboDecoder(const Turbo::Structure& codeStructure);
      TurboDecoder() = default;
      
      inline const Turbo::Structure& structure() const {return structure_;}
      
      virtual void decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg) = 0;
      virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output) = 0;
      
      std::vector<std::unique_ptr<MapDecoder>> code_;
      
      std::vector<double> extrinsic_;
      std::vector<double> extrinsicBuffer_;
      std::vector<double> parityIn_;
      std::vector<double> parityOut_;
      
    private:
      Turbo::Structure structure_;
    };
    
  }
  
}

#endif
