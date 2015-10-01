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

#ifndef TO_BITFIELD_H
#define TO_BITFIELD_H

#include <type_traits>

#include <mex.h>

#include "Structure/BitField.h"
#include "MexConversion.h"

template <typename T> struct MexType<fec::BitField<T>> {using ID = typename MexType<T>::ID; using isScalar = std::true_type;};

namespace std {
  template< class T >
  struct is_arithmetic<fec::BitField<T>> : std::true_type {};
}
  
#endif
