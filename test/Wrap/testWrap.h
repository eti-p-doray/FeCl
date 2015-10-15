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

#ifndef WRAP_FCN
#define WRAP_FCN

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "Convolutional/Convolutional.h"
#include "Convolutional/PuncturedConvolutional.h"
#include "Turbo/Turbo.h"
#include "Turbo/PuncturedTurbo.h"
#include "Ldpc/Ldpc.h"
#include "Ldpc/PuncturedLdpc.h"
#include "Serialization.h"

using namespace fec;

const DerivedTypeHolder<Convolutional,PuncturedConvolutional,Turbo,PuncturedTurbo,Ldpc,PuncturedLdpc> derivedCodec = {};

template <class Wrap>
class TestWrapFcn {
public:
  template <typename T> using wrapTo = typename Wrap::template wrapTo<T>;
  template <typename... Args> static auto toWrap(Args&&... args) -> decltype(Wrap::toWrap(std::forward<Args>(args)...)) {return Wrap::toWrap(std::forward<Args>(args)...);}
  template <typename T> using Allocator = typename Wrap::template Allocator<T>;
  template <typename T> using Handle = typename Wrap::template Handle<T>;
  using InArgList = typename Wrap::InArgList;
  using OutArgList = typename Wrap::OutArgList;
  
  using Signature = void(const InArgList in, OutArgList out);
  static const std::vector<std::function<Signature>> list;
};

template <class Wrap>
const std::vector<std::function<typename TestWrapFcn<Wrap>::Signature>> TestWrapFcn<Wrap>::list = {
  [](const InArgList in, OutArgList out) //scalarConversion
  {
    out[0] = toWrap(wrapTo<double>::f(in[0]));
  },
  [](const InArgList in, OutArgList out) //vectorConversion
  {
    out[0] = toWrap(wrapTo<std::vector<double>>::f(in[0]));
  },
};

#endif
