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
BOOST_CLASS_EXPORT_IMPLEMENT(Codec::detail::Structure);

Codec::Codec(std::unique_ptr<Codec::detail::Structure>&& structure, int workGroupSize) : structure_(std::move(structure))
{
  workGroupSize_ = workGroupSize;
}

/**
 *  Checks several blocs of msg bits.
 *  \param  parityIt Input iterator pointing to the first element in the parity bit sequence.
 */
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

/**
 *  Encodes several blocs of msg bits.
 *  \param  messageIt  Input iterator pointing to the first element in the msg bit sequence.
 *  \param  parityIt[out] Output iterator pointing to the first element in the parity bit sequence.
 *    The output neeeds to be pre-allocated.
 */
void Codec::encodeBlocks(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    structure().encode(msg, parity);
    msg += msgSize();
    parity += paritySize();
  }
}

std::vector<std::thread> Codec::createWorkGroup() const
{
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(getWorkGroupSize());
  return threadGroup;
}

size_t Codec::taskSize(size_t blockCount) const
{
  int n = std::thread::hardware_concurrency();
  if (n > getWorkGroupSize() || n == 0) {
    n = getWorkGroupSize();
  }
  return (blockCount+n-1)/n;
}
