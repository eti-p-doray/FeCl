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

#ifndef FEC_VITERBI_DECODER_H
#define FEC_VITERBI_DECODER_H

#include <vector>
#include <memory>

#include "../Convolutional.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the implementation of the viterbi decoder.
     *  This algorithm is used for simple decoding in a ConvolutionalCodec.
     */
    class ViterbiDecoder
    {
    public:
      static std::unique_ptr<ViterbiDecoder> create(const Convolutional::Structure&); /**< Creating function */
      ~ViterbiDecoder() = default;
      
      void decodeBlocks(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n);
      virtual void decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg) = 0;
      
    protected:
      ViterbiDecoder(const Convolutional::Structure&);
      
      inline const Convolutional::Structure& structure() const {return structure_;}
      
    private:
      const Convolutional::Structure structure_;
    };
    
  }
  
}

#endif
