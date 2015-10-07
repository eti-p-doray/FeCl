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

#include "Codec.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Codec);
BOOST_CLASS_EXPORT_IMPLEMENT(Codec::Structure);

Codec::Codec(std::unique_ptr<Codec::Structure>&& structure, int workGroupSize) : structure_(std::move(structure))
{
  workGroupSize_ = workGroupSize;
}

bool Codec::checkBlocks(std::vector<BitField<size_t>>::const_iterator parity, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    bool check = structure().check(parity);
    if (!check) {
      return false;
    }
    parity += paritySize();
  }
  return true;
}

void Codec::encodeBlocks(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    structure().encode(msg, parity);
    msg += msgSize();
    parity += paritySize();
  }
}
