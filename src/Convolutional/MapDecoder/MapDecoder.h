/*******************************************************************************
 This file is part of C3sar.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 C3sar is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 C3sar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_MAP_DECODER_H
#define FEC_MAP_DECODER_H

#include <vector>
#include <memory>

#include "../Convolutional.h"

namespace fec {

  /**
   *  This class contains the abstract implementation of the map decoder.
   *  This algorithm is used for decoding with a-priori information
   *  in a ConvolutionalCodec.
   *  The reason for this class is to offer an common interface of map decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
class MapDecoder
{
public:  
  static std::unique_ptr<MapDecoder> create(const Convolutional::Structure&); /**< Creating function */
  virtual ~MapDecoder() = default; /**< Default destructor */
  
  void soDecodeBlocks(Codec::InputIterator input, Codec::OutputIterator output, size_t n);
  virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output) = 0;
  
protected:
  MapDecoder(const Convolutional::Structure&); /**< Constructor */
  
  inline const Convolutional::Structure& structure() const {return structure_;} /**< Access the code structure */

private:
  const Convolutional::Structure structure_;
};
  
}

#endif
