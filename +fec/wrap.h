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

#include <mex.h>

#include "MexConversion.h"

enum WrapFcnId : uint32_t {
  Codec_destroy_id,
  Codec_save_id,
  Codec_load_id,
  Codec_encode_id,
  Codec_decode_id,
  Codec_soDecode_id,
  Codec_get_msgSize_id,
  Codec_get_systSize_id,
  Codec_get_stateSize_id,
  Codec_get_paritySize_id,
  Codec_get_workGroupSize_id,
  Codec_set_workGroupSize_id,
  Turbo_constructor_id,
  Turbo_get_decoderOptions_id,
  Turbo_set_decoderOptions_id,
  Turbo_set_encoderOptions_id,
};

void Codec_destroy( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_save( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_load( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_encode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_decode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_soDecode( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_get_msgSize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_get_systSize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_get_stateSize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_get_paritySize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_get_workGroupSize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Codec_set_workGroupSize( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );

void Turbo_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Turbo_get_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Turbo_set_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Turbo_set_encoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );

