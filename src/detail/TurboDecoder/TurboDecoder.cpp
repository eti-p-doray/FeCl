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

#include "TurboDecoder.h"
#include "TurboDecoderImpl.h"

using namespace fec;
using namespace fec::detail;

std::unique_ptr<TurboDecoder> TurboDecoder::create(const Turbo::Structure& structure)
{
  return std::unique_ptr<TurboDecoder>(new TurboDecoderImpl(structure));
}

TurboDecoder::TurboDecoder(const Turbo::Structure& structure) : structure_(structure)
{
  for (size_t i = 0; i < this->structure().constituentCount(); ++i) {
    code_.push_back(MapDecoder::create(this->structure().constituent(i)));
  }
  state_.resize(this->structure().stateSize());
  stateBuffer_.resize(this->structure().stateSize());;
  parityIn_.resize(this->structure().paritySize());
  parityOut_.resize(this->structure().paritySize());
}

void TurboDecoder::decodeBlocks(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last, std::vector<BitField<size_t>>::iterator output)
{
  while (first != last) {
    decodeBlock(first, output);
    first+= structure().paritySize(); output+=structure().msgSize();
  }
}

void TurboDecoder::soDecodeBlocks(Codec::const_iterator<double> first, Codec::const_iterator<double> last, Codec::iterator<double> output)
{
  while (first != last) {
    soDecodeBlock(first++, output++);
  }
}