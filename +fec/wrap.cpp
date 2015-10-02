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

#include "wrap.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  WrapFcnId fcnId = mxArrayTo<WrapFcnId>::f(prhs[0]);
  nrhs -= 1;
  prhs += 1;
  switch (fcnId) {
    case Codec_destroy_id:
      Codec_destroy(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_save_id:
      Codec_save(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_load_id:
      Codec_load(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_encode_id:
      Codec_encode(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_decode_id:
      Codec_decode(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_soDecode_id:
      Codec_soDecode(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_get_msgSize_id:
      Codec_get_msgSize(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_get_systSize_id:
      Codec_get_systSize(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_get_stateSize_id:
      Codec_get_stateSize(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_get_paritySize_id:
      Codec_get_paritySize(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_get_workGroupSize_id:
      Codec_get_workGroupSize(nlhs, plhs, nrhs, prhs);
      break;
    case Codec_set_workGroupSize_id:
      Codec_set_workGroupSize(nlhs, plhs, nrhs, prhs);
      break;
    case Turbo_constructor_id:
      Turbo_constructor(nlhs, plhs, nrhs, prhs);
      break;
    case Turbo_get_decoderOptions_id:
      Turbo_get_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Turbo_set_decoderOptions_id:
      Turbo_set_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Turbo_set_encoderOptions_id:
      Turbo_set_encoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Turbo_createPermutation_id:
      Turbo_createPermutation(nlhs, plhs, nrhs, prhs);
      break;
    case Ldpc_constructor_id:
      Ldpc_constructor(nlhs, plhs, nrhs, prhs);
      break;
    case Ldpc_get_decoderOptions_id:
      Ldpc_get_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Ldpc_set_decoderOptions_id:
      Ldpc_set_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Ldpc_set_encoderOptions_id:
      Ldpc_set_encoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Convolutional_constructor_id:
      Convolutional_constructor(nlhs, plhs, nrhs, prhs);
      break;
    case Convolutional_get_decoderOptions_id:
      Convolutional_get_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Convolutional_set_decoderOptions_id:
      Convolutional_set_decoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Convolutional_set_encoderOptions_id:
      Convolutional_set_encoderOptions(nlhs, plhs, nrhs, prhs);
      break;
    case Convolutional_createPermutation_id:
      Convolutional_createPermutation(nlhs, plhs, nrhs, prhs);
      break;
    case Trellis_constructor_id:
      Trellis_constructor(nlhs, plhs, nrhs, prhs);
      break;
    default:
      throw  std::invalid_argument("Unknow fonction id");
      break;
  }
}