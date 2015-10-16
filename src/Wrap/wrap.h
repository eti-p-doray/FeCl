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
class WrapFcn {
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
const std::vector<std::function<typename WrapFcn<Wrap>::Signature>> WrapFcn<Wrap>::list = {
  [](const InArgList in, OutArgList out) //Codec_destroy
  {
    try {
      auto codec = wrapTo<Handle<Codec>>::f(in[0]);
      codec.reset();
      out[0] = toWrap(std::move(codec));
    } catch (...) {
      Handle<fec::Codec> null;
      out[0] = toWrap(std::move(null));
    }
  },
  
  [](const InArgList in, OutArgList out) //Codec_save
  {
    auto codec = wrapTo<Handle<Codec>>::f(in[0]);
    out[0] = save(codec, derivedCodec);
  },
  
  [](const InArgList in, OutArgList out) //Codec_load
  {
    Handle<Codec> codec = load<fec::Codec>(in[0], derivedCodec);
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Codec_check
  {
    auto codec = wrapTo<Handle<Codec>>::f(in[0]);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>> parity;
    try {
      parity = wrapTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>>>::f(in[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Parity vector is invalid");
    }
    out[0] = toWrap(codec->check(parity));
  },
  
  [](const InArgList in, OutArgList out) //Codec_encode
  {
    auto codec = wrapTo<Handle<Codec>>::f(in[0]);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>> msg;
    try {
      msg = wrapTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>>>::f(in[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Msg vector is invalid");
    }
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>> parity;
    codec->encode(msg, parity);
    out[0] = toWrap(parity);
  },
  
  [](const InArgList in, OutArgList out) //Codec_decode
  {
    auto codec = wrapTo<Handle<Codec>>::f(in[0]);
    
    std::vector<LlrType,Allocator<LlrType>> parity;
    try {
      parity = wrapTo<std::vector<LlrType,Allocator<LlrType>>>::f(in[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Parity vector is invalid");
    }
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>> msg;
    codec->decode(parity, msg);
    out[0] = toWrap(msg);
  },
  
  [](const InArgList in, OutArgList out) //Codec_soDecode
  {
    auto codec = wrapTo<Handle<Codec>>::f(in[0]);
    
    std::vector<LlrType,Allocator<LlrType>> parityIn = wrapTo<std::vector<LlrType,Allocator<LlrType>>>::f(in[1]);
    std::vector<LlrType,Allocator<LlrType>> stateIn;
    if (in.size() > 2) stateIn = wrapTo<std::vector<LlrType,Allocator<LlrType>>>::f(in[2]);
    std::vector<LlrType,Allocator<LlrType>> systIn;
    if (in.size() > 3) systIn = wrapTo<std::vector<LlrType,Allocator<LlrType>>>::f(in[3]);
    
    auto input = Codec::Info<const std::vector<LlrType,Allocator<LlrType>>>();
    if (parityIn.size()) input.parity(parityIn);
    if (stateIn.size()) input.state(stateIn);
    if (systIn.size()) input.syst(systIn);
    
    auto output = Codec::Info<std::vector<LlrType,Allocator<LlrType>>>();
    std::vector<LlrType,Allocator<LlrType>> msg;
    std::vector<LlrType,Allocator<LlrType>> systOut;
    std::vector<LlrType,Allocator<LlrType>> stateOut;
    std::vector<LlrType,Allocator<LlrType>> parityOut;
    output.msg(msg);
    if (out.size() > 1) output.syst(systOut);
    if (out.size() > 2) output.state(stateOut);
    if (out.size() > 3) output.parity(parityOut);
    codec->soDecode(input, output);
    
    out[0] = toWrap(msg);
    if (out.size() > 1) out[1] = toWrap(systOut);
    if (out.size() > 2) out[2] = toWrap(stateOut);
    if (out.size() > 3) out[3] = toWrap(parityOut);
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_msgSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>::f(in[0])->msgSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_systSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>::f(in[0])->systSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_stateSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>::f(in[0])->stateSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_paritySize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>::f(in[0])->paritySize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_workGroupSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>::f(in[0])->getWorkGroupSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_set_workGroupSize
  {
    wrapTo<Handle<Codec>>::f(in[0])->setWorkGroupSize(wrapTo<int>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_constructor
  {
    Turbo::Structure structure(wrapTo<Turbo::EncoderOptions>::f(in[0]), wrapTo<Turbo::DecoderOptions>::f(in[1]));
    Handle<Codec> codec(new Turbo(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Turbo>>::f(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Turbo_setDecoderOptions
  {
    wrapTo<Handle<Turbo>>::f(in[0])->setDecoderOptions(wrapTo<Turbo::DecoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_setEncoderOptions
  {
    wrapTo<Handle<Turbo>>::f(in[0])->setEncoderOptions(wrapTo<Turbo::EncoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Turbo>>::f(in[0])->puncturing(wrapTo<Turbo::PunctureOptions>::f(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_Lte3Gpp_interleaver
  {
    out[0] = toWrap(Turbo::Lte3Gpp::interleaver(wrapTo<size_t>::f(in[0])));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedTurbo_constructor
  {
    PuncturedTurbo::Structure structure(wrapTo<Turbo::EncoderOptions>::f(in[0]), wrapTo<Turbo::PunctureOptions>::f(in[1]), wrapTo<Turbo::DecoderOptions>::f(in[2]));
    Handle<Codec> codec(new PuncturedTurbo(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedTurbo_set_punctureOptions
  {
    wrapTo<Handle<PuncturedTurbo>>::f(in[0])->setPunctureOptions(wrapTo<Turbo::PunctureOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_constructor
  {
    
    Ldpc::Structure structure(wrapTo<Ldpc::EncoderOptions>::f(in[0]), wrapTo<Ldpc::DecoderOptions>::f(in[1]));
    Handle<Codec> codec(new Ldpc(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Ldpc>>::f(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_setDecoderOptions
  {
    wrapTo<Handle<Ldpc>>::f(in[0])->setDecoderOptions(wrapTo<Ldpc::DecoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_setEncoderOptions
  {
    wrapTo<Handle<Ldpc>>::f(in[0])->setEncoderOptions(wrapTo<Ldpc::EncoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Ldpc>>::f(in[0])->puncturing(wrapTo<Ldpc::PunctureOptions>::f(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_DvbS2_matrix
  {
    out[0] = toWrap(Ldpc::DvbS2::matrix(wrapTo<size_t>::f(in[0]), wrapTo<double>::f(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedLdpc_constructor
  {
    
    PuncturedLdpc::Structure structure(wrapTo<Ldpc::EncoderOptions>::f(in[0]), wrapTo<Ldpc::PunctureOptions>::f(in[1]), wrapTo<Ldpc::DecoderOptions>::f(in[2]));
    Handle<Codec> codec(new PuncturedLdpc(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedLdpc_set_punctureOptions
  {
    wrapTo<Handle<PuncturedLdpc>>::f(in[0])->setPunctureOptions(wrapTo<Ldpc::PunctureOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_constructor
  {
    
    Convolutional::Structure structure(wrapTo<Convolutional::EncoderOptions>::f(in[0]), wrapTo<Convolutional::DecoderOptions>::f(in[1]));
    Handle<Codec> codec(new Convolutional(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Convolutional>>::f(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_setDecoderOptions
  {
    wrapTo<Handle<Convolutional>>::f(in[0])->setDecoderOptions(wrapTo<Convolutional::DecoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_setEncoderOptions
  {
    wrapTo<Handle<Convolutional>>::f(in[0])->setEncoderOptions(wrapTo<Convolutional::EncoderOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Convolutional>>::f(in[0])->puncturing(wrapTo<Convolutional::PunctureOptions>::f(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedLdpc_constructor
  {
    PuncturedConvolutional::Structure structure(wrapTo<Convolutional::EncoderOptions>::f(in[0]), wrapTo<Convolutional::PunctureOptions>::f(in[1]), wrapTo<Convolutional::DecoderOptions>::f(in[2]));
    Handle<Codec> codec(new PuncturedConvolutional(structure));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //PuncturedLdpc_set_punctureOptions
  {
    wrapTo<Handle<PuncturedConvolutional>>::f(in[0])->setPunctureOptions(wrapTo<Convolutional::PunctureOptions>::f(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Trellis_constructor
  {
    auto constraintLength = wrapTo<std::vector<BitField<size_t>>>::f(in[0]);
    auto generator = wrapTo<std::vector<std::vector<BitField<size_t>>>>::f(in[1]);
    auto feedback = wrapTo<std::vector<BitField<size_t>>>::f(in[2]);
    
    out[0] = toWrap(Trellis(constraintLength, generator, feedback));
  },
};

#endif
