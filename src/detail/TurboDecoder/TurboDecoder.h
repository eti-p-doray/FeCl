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
      
      void decodeBlocks(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last, std::vector<BitField<size_t>>::iterator output);
      void soDecodeBlocks(Codec::const_iterator<double> first, Codec::const_iterator<double> last, Codec::iterator<double> output);
      
      virtual void decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg) = 0;
      virtual void soDecodeBlock(Codec::const_iterator<double> input, Codec::iterator<double> output) = 0;
      
    protected:
      TurboDecoder(const Turbo::Structure& codeStructure);
      TurboDecoder() = default;
      
      inline const Turbo::Structure& structure() const {return structure_;}
      
      std::vector<std::unique_ptr<MapDecoder>> code_;
      
      std::vector<double> state_;
      std::vector<double> stateBuffer_;
      std::vector<double> parityIn_;
      std::vector<double> parityOut_;
      
    private:
      Turbo::Structure structure_;
    };
    
  }
  
}

#endif
