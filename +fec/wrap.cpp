/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    default:
      throw  std::invalid_argument("Unknow fonction id");
      break;
  }
}