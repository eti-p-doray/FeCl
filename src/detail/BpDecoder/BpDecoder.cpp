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

#include "BpDecoder.h"
#include "BpDecoderImpl.h"

using namespace fec::detail;

std::unique_ptr<BpDecoder> BpDecoder::create(const Ldpc::Structure& structure)
{
  switch (structure.decoderAlgorithm()) {
    default:
    case Exact:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<FloatLlrMetrics,BoxSum>(structure));
      break;
      
    case Linear:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<FloatLlrMetrics,LinearBoxSum>(structure));
      break;
      
    case Approximate:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<FloatLlrMetrics,MinBoxSum>(structure));
      break;
  }
}

void BpDecoder::decodeBlocks(std::vector<double>::const_iterator parityf, std::vector<double>::const_iterator parityl, std::vector<BitField<size_t>>::iterator msg)
{
  while (parityf != parityl) {
    decodeBlock(parityf, msg);
    parityf += structure().paritySize();
    msg += structure().msgSize();
  }
}

void BpDecoder::soDecodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<double> output)
{
  while (first != last) {
    soDecodeBlock(first++, output++);
  }
}

BpDecoder::BpDecoder(const Ldpc::Structure& structure) : structure_(structure)
{
}
