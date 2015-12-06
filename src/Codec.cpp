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

Codec::Codec(std::unique_ptr<detail::Codec::Structure>&& structure, int workGroupSize) : structure_(std::move(structure)), workGroup_(workGroupSize)
{
}

/**
 *  Checks several blocs of msg bits.
 *  \param  parityIt Input iterator pointing to the first element in the parity bit sequence.
 */
bool Codec::checkBlocks(detail::Codec::const_iterator<BitField<size_t>> first, detail::Codec::const_iterator<BitField<size_t>> last) const
{
  while (first != last) {
    bool check = structure().check(first.at(detail::Codec::Parity));
    if (!check) {
      return false;
    }
    ++first;
  }
  return true;
}

/**
 *  Encodes several blocs of msg bits.
 *  \param  messageIt  Input iterator pointing to the first element in the msg bit sequence.
 *  \param  parityIt[out] Output iterator pointing to the first element in the parity bit sequence.
 *    The output neeeds to be pre-allocated.
 */
void Codec::encodeBlocks(detail::Codec::const_iterator<BitField<size_t>> first, detail::Codec::const_iterator<BitField<size_t>> last, detail::Codec::iterator<BitField<size_t>> output) const
{
  while (first != last) {
    structure().encode(first.at(detail::Codec::Msg), output.at(detail::Codec::Parity));
    ++first; ++output;
  }
}

