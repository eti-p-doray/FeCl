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

#ifndef MEX_SAVE_LOAD
#define MEX_SAVE_LOAD

#include <memory>
#include <cstring>
#include <type_traits>
#include <vector>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <mex.h>

#include "Structure/Serialization.h"
#include "MexConversion.h"

template <class T, class DerivedTypeHolder>
mxArray* save(const MexHandle<T>& u,  DerivedTypeHolder derived)
{
  const T* base_pointer = u.get();
  
  size_t size = archiveSize(base_pointer, derived);
  mxArray* out = mxCreateNumericMatrix(size, 1, MexType<uint8_t>::ID::value, mxREAL);
  save(base_pointer, reinterpret_cast<char*>(mxGetData(out)), size, derived);
  
  return out;
}

template <class T, class DerivedTypeHolder>
MexHandle<T> load(const mxArray* in, DerivedTypeHolder derived)
{
  if (in == nullptr) {
    std::invalid_argument("null");
  }
  if (mxGetData(in) == nullptr) {
    std::invalid_argument("invalid");
  }
  auto ptr = load<T>(reinterpret_cast<const char*>(mxGetData(in)), mxGetNumberOfElements(in), derived);
  return MexHandle<T>(ptr.release());
}

#endif
