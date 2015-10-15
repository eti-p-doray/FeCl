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

#include "../testWrap.h"

#include "util/Conversion.h"
#include "util/MexHandle.h"
#include "util/MexAllocator.h"
#include "util/BitField.h"
#include "util/BitMatrix.h"
#include "util/Permutation.h"
#include "util/SaveLoad.h"
#include "util/Trellis.h"
#include "util/MexArgList.h"

#include "Turbo/EncoderOptions.h"
#include "Turbo/DecoderOptions.h"
#include "Turbo/PunctureOptions.h"

#include "Ldpc/EncoderOptions.h"
#include "Ldpc/DecoderOptions.h"
#include "Ldpc/PunctureOptions.h"

#include "Convolutional/EncoderOptions.h"
#include "Convolutional/DecoderOptions.h"
#include "Convolutional/PunctureOptions.h"

class MatlabWrap {
public:
  template <typename T> using wrapTo = mxArrayTo<T>;
  template <typename... Args> static auto toWrap(Args&&... args) -> decltype(toMxArray(std::forward<Args>(args)...)) {return toMxArray(std::forward<Args>(args)...);}
  template <typename T> using Allocator = typename MexAllocatorHolder<T>::type;
  template <typename T> using Handle = MexHandle<T>;
  using InArgList = MexArgList<const mxArray*>;
  using OutArgList = MexArgList<mxArray*>;
};

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  size_t fcnId = mxArrayTo<size_t>::f(prhs[0]);
  nrhs -= 1;
  prhs += 1;
  if (fcnId > TestWrapFcn<MatlabWrap>::list.size()) {
    throw  std::invalid_argument("Unknown fonction id");
  }
  TestWrapFcn<MatlabWrap>::list[fcnId](MexArgList<const mxArray*>(nrhs, prhs), MexArgList<mxArray*>(nlhs, plhs));
}