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

#ifndef FEC_BP_DECODER_H
#define FEC_BP_DECODER_H

#include <vector>
#include <memory>

#include "../Ldpc.h"

namespace fec {

  /**
   *  This class contains the abstract implementation of the belief propagation decoder.
   *  This algorithm is used for decoding in an LdpcCodec.
   *  The reason for this class is to offer an common interface of bp decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
class BpDecoder
{
public:
  static std::unique_ptr<BpDecoder> create(const Ldpc::Structure&);
  virtual ~BpDecoder() = default;
  
  void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n);
  void soDecodeBlocks(Codec::InputIterator input, Codec::OutputIterator output, size_t n);
  
protected:
  BpDecoder(const Ldpc::Structure& codeStructure);
  
  virtual void decodeBlock(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg) = 0;
  virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output) = 0;
  
  inline const Ldpc::Structure& structure() const {return structure_;}

private:
  
  const Ldpc::Structure structure_;
};
  
}

#endif
