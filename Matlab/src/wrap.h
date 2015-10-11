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

#ifndef WRAP
#define WRAP

#include <mex.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "Convolutional/Convolutional.h"
#include "Convolutional/PuncturedConvolutional.h"
#include "Turbo/Turbo.h"
#include "Turbo/PuncturedTurbo.h"
#include "Ldpc/Ldpc.h"
#include "Ldpc/PuncturedLdpc.h"

#include "util/WrapConversion.h"
#include "util/WrapHandle.h"
#include "util/WrapAllocator.h"
#include "util/BitField.h"
#include "util/BitMatrix.h"
#include "util/Permutation.h"
#include "util/SaveLoad.h"
#include "util/Trellis.h"

#include "Turbo/EncoderOptions.h"
#include "Turbo/DecoderOptions.h"
#include "Turbo/PunctureOptions.h"

#include "Ldpc/EncoderOptions.h"
#include "Ldpc/DecoderOptions.h"
#include "Ldpc/PunctureOptions.h"

#include "Convolutional/EncoderOptions.h"
#include "Convolutional/DecoderOptions.h"
#include "Convolutional/PunctureOptions.h"

using namespace fec;

using WrapFcn = void(int, mxArray_tag **, int, const mxArray_tag **);
const DerivedTypeHolder<Convolutional,PuncturedConvolutional,Turbo,PuncturedTurbo,Ldpc,PuncturedLdpc> derivedCodec = {};

const std::vector<std::function<WrapFcn>> wrapFcns = {
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_destroy
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    try {
      WrapHandle<Codec> codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
      codec.reset();
      plhs[0] = toMxArray(std::move(codec));
    } catch (...) {
      WrapHandle<fec::Codec> null;
      plhs[0] = toMxArray(std::move(null));
    }
    mexUnlock();
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_save
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    auto codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
    plhs[0] = save(codec, derivedCodec);
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_load
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    WrapHandle<Codec> codec = load<fec::Codec>(prhs[0], derivedCodec);
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_check
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    
    auto codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type> parity;
    try {
      parity = mxArrayTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type>>::f(prhs[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Parity vector is invalid");
    }
    plhs[0] = toMxArray(codec->check(parity));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_encode
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    
    auto codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
    
    std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type> msg;
    try {
      msg = mxArrayTo<std::vector<BitField<size_t>,Allocator<BitField<size_t>>::type>>::f(prhs[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Msg vector is invalid");
    }
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>::type> parity;
    codec->encode(msg, parity);
    plhs[0] = toMxArray(parity);
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_decode
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    
    auto codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
    
    std::vector<LlrType,Allocator<LlrType>::type> parity;
    try {
      parity = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[1]);
    } catch (std::exception& e) {
      throw std::invalid_argument("Parity vector is invalid");
    }
    std::vector<BitField<size_t>, Allocator<BitField<size_t>>::type> msg;
    codec->decode(parity, msg);
    plhs[0] = toMxArray(msg);
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_soDecode
  {
    
    auto codec = mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec);
    
    std::vector<LlrType,Allocator<LlrType>::type> parityIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[1]);
    std::vector<LlrType,Allocator<LlrType>::type> stateIn;
    if (nrhs > 2) stateIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[2]);
    std::vector<LlrType,Allocator<LlrType>::type> systIn;
    if (nrhs > 3) systIn = mxArrayTo<std::vector<LlrType,Allocator<LlrType>::type>>::f(prhs[3]);
    
    auto input = Codec::Info<const std::vector<LlrType,Allocator<LlrType>::type>>();
    if (parityIn.size()) input.parity(parityIn);
    if (stateIn.size()) input.state(stateIn);
    if (systIn.size()) input.syst(systIn);
    
    auto output = Codec::Info<std::vector<LlrType,Allocator<LlrType>::type>>();
    std::vector<LlrType,Allocator<LlrType>::type> msg;
    std::vector<LlrType,Allocator<LlrType>::type> systOut;
    std::vector<LlrType,Allocator<LlrType>::type> stateOut;
    std::vector<LlrType,Allocator<LlrType>::type> parityOut;
    output.msg(msg);
    if (nlhs > 1) output.syst(systOut);
    if (nlhs > 2) output.state(stateOut);
    if (nlhs > 3) output.parity(parityOut);
    codec->soDecode(input, output);
    
    plhs[0] = toMxArray(msg);
    if (nlhs > 1) plhs[1] = toMxArray(systOut);
    if (nlhs > 2) plhs[2] = toMxArray(stateOut);
    if (nlhs > 3) plhs[3] = toMxArray(parityOut);
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_get_msgSize
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->msgSize());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_get_systSize
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->systSize());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_get_stateSize
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->stateSize());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_get_paritySize
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->paritySize());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_get_workGroupSize
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->getWorkGroupSize());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Codec_set_workGroupSize
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Codec>>::f(prhs[0], derivedCodec)->setWorkGroupSize(mxArrayTo<int>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 2, 1);
    Turbo::Structure structure(mxArrayTo<Turbo::EncoderOptions>::f(prhs[0]), mxArrayTo<Turbo::DecoderOptions>::f(prhs[1]));
    WrapHandle<Codec> codec(new Turbo(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_getDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Turbo>>::f(prhs[0])->getDecoderOptions());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_setDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Turbo>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Turbo::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_setEncoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Turbo>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Turbo::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_createPermutation
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Turbo>>::f(prhs[0])->createPermutation(mxArrayTo<Turbo::PunctureOptions>::f(prhs[1])));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Turbo_Lte3Gpp_interleaver
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(Turbo::Lte3Gpp::interleaver(mxArrayTo<size_t>::f(prhs[0])));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedTurbo_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 3, 1);
    PuncturedTurbo::Structure structure(mxArrayTo<Turbo::EncoderOptions>::f(prhs[0]), mxArrayTo<Turbo::PunctureOptions>::f(prhs[1]), mxArrayTo<Turbo::DecoderOptions>::f(prhs[2]));
    WrapHandle<Codec> codec(new PuncturedTurbo(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedTurbo_set_punctureOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<PuncturedTurbo>>::f(prhs[0])->setPunctureOptions(mxArrayTo<Turbo::PunctureOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 2, 1);
    Ldpc::Structure structure(mxArrayTo<Ldpc::EncoderOptions>::f(prhs[0]), mxArrayTo<Ldpc::DecoderOptions>::f(prhs[1]));
    WrapHandle<Codec> codec(new Ldpc(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_getDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Ldpc>>::f(prhs[0])->getDecoderOptions());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_setDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Ldpc>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Ldpc::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_setEncoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Ldpc>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Ldpc::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_createPermutation
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Ldpc>>::f(prhs[0])->createPermutation(mxArrayTo<Ldpc::PunctureOptions>::f(prhs[1])));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Ldpc_DvbS2_matrix
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    plhs[0] = toMxArray(Ldpc::DvbS2::matrix(mxArrayTo<size_t>::f(prhs[0]), mxArrayTo<double>::f(prhs[1])));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedLdpc_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 3, 1);
    PuncturedLdpc::Structure structure(mxArrayTo<Ldpc::EncoderOptions>::f(prhs[0]), mxArrayTo<Ldpc::PunctureOptions>::f(prhs[1]), mxArrayTo<Ldpc::DecoderOptions>::f(prhs[2]));
    WrapHandle<Codec> codec(new PuncturedLdpc(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedLdpc_set_punctureOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<PuncturedLdpc>>::f(prhs[0])->setPunctureOptions(mxArrayTo<Ldpc::PunctureOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Convolutional_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 2, 1);
    Convolutional::Structure structure(mxArrayTo<Convolutional::EncoderOptions>::f(prhs[0]), mxArrayTo<Convolutional::DecoderOptions>::f(prhs[1]));
    WrapHandle<Codec> codec(new Convolutional(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Convolutional_getDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 1, 1);
    plhs[0] = toMxArray(mxArrayTo<WrapHandle<Convolutional>>::f(prhs[0])->getDecoderOptions());
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Convolutional_setDecoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Convolutional>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Convolutional::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Convolutional_setEncoderOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<Convolutional>>::f(prhs[0])->setEncoderOptions(mxArrayTo<Convolutional::EncoderOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Convolutional_createPermutation
  {
    checkArgCount(nlhs, nrhs, 2, 1);
    plhs[0]  = toMxArray(mxArrayTo<WrapHandle<Convolutional>>::f(prhs[0])->createPermutation(mxArrayTo<Convolutional::PunctureOptions>::f(prhs[1])));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedLdpc_constructor
  {
    mexLock();
    checkArgCount(nlhs, nrhs, 3, 1);
    PuncturedConvolutional::Structure structure(mxArrayTo<Convolutional::EncoderOptions>::f(prhs[0]), mxArrayTo<Convolutional::PunctureOptions>::f(prhs[1]), mxArrayTo<Convolutional::DecoderOptions>::f(prhs[2]));
    WrapHandle<Codec> codec(new PuncturedConvolutional(structure));
    plhs[0] = toMxArray(std::move(codec));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //PuncturedLdpc_set_punctureOptions
  {
    checkArgCount(nlhs, nrhs, 2, 0);
    mxArrayTo<WrapHandle<PuncturedConvolutional>>::f(prhs[0])->setPunctureOptions(mxArrayTo<Convolutional::PunctureOptions>::f(prhs[1]));
  },
  
  [](int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) //Trellis_constructor
  {
    checkArgCount(nlhs, nrhs, 3, 1);
    auto constraintLength = mxArrayTo<std::vector<BitField<size_t>>>::f(prhs[0]);
    auto generator = mxArrayTo<std::vector<std::vector<BitField<size_t>>>>::f(prhs[1]);
    auto feedback = mxArrayTo<std::vector<BitField<size_t>>>::f(prhs[2]);
    
    plhs[0] = toMxArray(Trellis(constraintLength, generator, feedback));
  },
};

#endif
