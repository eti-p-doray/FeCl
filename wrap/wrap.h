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

#include "Convolutional.h"
#include "Turbo.h"
#include "Ldpc.h"
#include "Serialization.h"

using namespace fec;

const detail::DerivedTypeHolder<Convolutional,Turbo,Ldpc> derivedCodec = {};

template <class Wrap>
class WrapFcn {
public:
  template <typename T> using wrapTo = typename Wrap::template wrapTo<T>;
  template <typename... Args> static auto toWrap(Args&&... args) -> decltype(Wrap::toWrap(std::forward<Args>(args)...)) {return Wrap::toWrap(std::forward<Args>(args)...);}
  template <typename T> using Allocator = typename Wrap::template Allocator<T>;
  template <typename T> using Handle = typename Wrap::template Handle<T>;
  using InArgList = typename Wrap::InArgList;
  using OutArgList = typename Wrap::OutArgList;
  template <typename T, typename... Args> static Handle<T> load(Args&&... args) {return Wrap::template load<T>(std::forward<Args>(args)...);}
  
  using Signature = void(const InArgList in, OutArgList out);
  static const std::vector<std::function<Signature>> list;
};

template <class Wrap>
const std::vector<std::function<typename WrapFcn<Wrap>::Signature>> WrapFcn<Wrap>::list = {
  [](const InArgList in, OutArgList out) //Codec_destroy
  {
    try {
      auto codec = wrapTo<Handle<Codec>>{}(in[0]);
      codec.reset();
      out[0] = toWrap(std::move(codec));
    } catch (...) {
      Handle<fec::Codec> null;
      out[0] = toWrap(std::move(null));
    }
  },
  
  [](const InArgList in, OutArgList out) //Codec_save
  {
    auto codec = wrapTo<Handle<Codec>>{}("Save codec")(in[0]);
    out[0] = save(codec, derivedCodec);
  },
  
  [](const InArgList in, OutArgList out) //Codec_load
  {
    Handle<Codec> codec = load<fec::Codec>(in[0], derivedCodec);
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Codec_check
  {
    auto codec = wrapTo<Handle<Codec>>{}("check")("Codec object")(in[0]);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>> parity = wrapTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>>>{}("Parity vector")(in[1]);
    out[0] = toWrap(codec->check(parity));
  },
  
  [](const InArgList in, OutArgList out) //Codec_encode
  {
    auto codec = wrapTo<Handle<Codec>>{}("encode")("Codec object")(in[0]);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>> msg = wrapTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>>>{}("Msg vector")("boubou")(in[1]);
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>> parity;
    codec->encode(msg, parity);
    out[0] = toWrap(parity);
  },
  
  [](const InArgList in, OutArgList out) //Codec_decode
  {
    auto codec = wrapTo<Handle<Codec>>{}("decode")("Codec object")(in[0]);
    
    std::vector<double,Allocator<double>> parity = wrapTo<std::vector<double,Allocator<double>>>{}("Parity vector")(in[1]);
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>> msg;
    codec->decode(parity, msg);
    out[0] = toWrap(msg);
  },
  
  [](const InArgList in, OutArgList out) //Codec_soDecode
  {
    auto codec = wrapTo<Handle<Codec>>{}("doDecode")("Codec object")(in[0]);
    
    std::vector<double,Allocator<double>> parityIn = wrapTo<std::vector<double,Allocator<double>>>{}("Parity vector")(in[1]);
    std::vector<double,Allocator<double>> stateIn;
    if (in.size() > 2) stateIn = wrapTo<std::vector<double,Allocator<double>>>{}("State vector")(in[2]);
    std::vector<double,Allocator<double>> systIn;
    if (in.size() > 3) systIn = wrapTo<std::vector<double,Allocator<double>>>{}("Syst vector")(in[3]);
    
    auto input = Codec::Input<Allocator>();
    if (parityIn.size()) input.parity(parityIn);
    if (stateIn.size()) input.state(stateIn);
    if (systIn.size()) input.syst(systIn);
    
    auto output = Codec::Output<Allocator>();
    std::vector<double,Allocator<double>> msg;
    std::vector<double,Allocator<double>> systOut;
    std::vector<double,Allocator<double>> stateOut;
    std::vector<double,Allocator<double>> parityOut;
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
    out[0] = toWrap(wrapTo<Handle<Codec>>{}(in[0])->msgSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_systSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>{}(in[0])->systSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_stateSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>{}(in[0])->stateSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_paritySize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>{}(in[0])->paritySize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_get_workGroupSize
  {
    out[0] = toWrap(wrapTo<Handle<Codec>>{}(in[0])->getWorkGroupSize());
  },
  
  [](const InArgList in, OutArgList out) //Codec_set_workGroupSize
  {
    wrapTo<Handle<Codec>>{}(in[0])->setWorkGroupSize(wrapTo<int>{}(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_constructor
  {
    Handle<Codec> codec(new Turbo(wrapTo<Turbo::EncoderOptions>{}("Encoder options")(in[0]), wrapTo<Turbo::DecoderOptions>{}("Decoder options")(in[1])));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Turbo>>{}("Codec object")(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Turbo_setDecoderOptions
  {
    wrapTo<Handle<Turbo>>{}(in[0])->setDecoderOptions(wrapTo<Turbo::DecoderOptions>{}("Decoder options")(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Turbo>>{}(in[0])->puncturing(wrapTo<Turbo::PunctureOptions>{}("Puncture options")(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Turbo_Lte3Gpp_interleaver
  {
    out[0] = toWrap(Turbo::Lte3Gpp::interleaver(wrapTo<size_t>{}("size")(in[0])));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_constructor
  {
    Handle<Codec> codec(new Ldpc(wrapTo<Ldpc::EncoderOptions>{}("Encoder options")(in[0]), wrapTo<Ldpc::DecoderOptions>{}("Decoder options")(in[1])));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Ldpc>>{}("Codec object")(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_setDecoderOptions
  {
    wrapTo<Handle<Ldpc>>{}(in[0])->setDecoderOptions(wrapTo<Ldpc::DecoderOptions>{}("Decoder options")(in[1]));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Ldpc>>{}(in[0])->puncturing(wrapTo<Ldpc::PunctureOptions>{}("Puncture options")(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Ldpc_DvbS2_matrix
  {
    out[0] = toWrap(Ldpc::DvbS2::matrix(wrapTo<size_t>{}("size")(in[0]), wrapTo<double>{}("rate")(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_constructor
  {
    Handle<Codec> codec(new Convolutional(wrapTo<Convolutional::EncoderOptions>{}("Encoder options")(in[0]), wrapTo<Convolutional::DecoderOptions>{}("Decoder options")(in[1])));
    out[0] = toWrap(std::move(codec));
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_getDecoderOptions
  {
    out[0] = toWrap(wrapTo<Handle<Convolutional>>{}("Codec object")(in[0])->getDecoderOptions());
  },
  
  [](const InArgList in, OutArgList out) //Convolutional_setDecoderOptions
  {
    wrapTo<Handle<Convolutional>>{}(in[0])->setDecoderOptions(wrapTo<Convolutional::DecoderOptions>{}("Decoder options")(in[1]));
  },
  
  /*[](const InArgList in, OutArgList out) //Convolutional_setEncoderOptions
  {
    wrapTo<Handle<Convolutional>>{}(in[0])->setEncoderOptions(wrapTo<Convolutional::EncoderOptions>{}(in[1]));
  },*/
  
  [](const InArgList in, OutArgList out) //Convolutional_puncturing
  {
    out[0] = toWrap(wrapTo<Handle<Convolutional>>{}(in[0])->puncturing(wrapTo<Convolutional::PunctureOptions>{}("Puncture options")(in[1])));
  },
  
  [](const InArgList in, OutArgList out) //Trellis_constructor
  {
    auto constraintLength = wrapTo<std::vector<size_t>>{}("constraint length")(in[0]);
    auto generator = wrapTo<std::vector<std::vector<BitField<size_t>>>>{}("generator")(in[1]);
    auto feedback = wrapTo<std::vector<BitField<size_t>>>{}("feedback")(in[2]);
    
    out[0] = toWrap(Trellis(constraintLength, generator, feedback));
  },
};

#endif
