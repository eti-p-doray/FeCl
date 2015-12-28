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

#ifndef FEC_BIT_ORDERING_H
#define FEC_BIT_ORDERING_H

namespace fec {

  /**
   *  Ordering of parity bit in Turbo.
   *  This defines the ordering of parity bits that are output from a Turbo permutation of a PuncturedTurbo Codec.
   */
  enum BitOrdering {
    Alternate,/**< Systematic and parity bits are alternated */
    Group,/**< Systematic bits are group together and parity bits from each constituents are grouped together. */
  };
  
}

#endif
