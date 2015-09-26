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

#include "TurboDecoder.h"
#include "TurboDecoderImpl.h"

using namespace fec;

std::unique_ptr<TurboDecoder> TurboDecoder::create(const Turbo::Structure& structure)
{
  return std::unique_ptr<TurboDecoder>(new TurboDecoderImpl(structure));
}

TurboDecoder::TurboDecoder(const Turbo::Structure& structure) : structure_(structure)
{
  for (size_t i = 0; i < this->structure().constituentCount(); ++i) {
    code_.push_back(MapDecoder::create(this->structure().constituent(i)));
  }
  extrinsic_.resize(this->structure().stateSize());
  extrinsicBuffer_.resize(this->structure().stateSize());;
  parityIn_.resize(this->structure().paritySize());
  parityOut_.resize(this->structure().paritySize());
}

void TurboDecoder::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    decodeBlock(parity, msg);
    parity += structure().paritySize();
    msg += structure().msgSize();
  }
}

void TurboDecoder::soDecodeBlocks(Codec::InputIterator input, Codec::OutputIterator output, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    soDecodeBlock(input, output);
    ++input;
    ++output;
  }
}