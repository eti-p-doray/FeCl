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
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <mex.h>

#include "util/MexConversion.h"

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
  Turbo_createPermutation_id,
  Ldpc_constructor_id,
  Ldpc_get_decoderOptions_id,
  Ldpc_set_decoderOptions_id,
  Ldpc_set_encoderOptions_id,
  Convolutional_constructor_id,
  Convolutional_get_decoderOptions_id,
  Convolutional_set_decoderOptions_id,
  Convolutional_set_encoderOptions_id,
  Trellis_constructor_id,
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
void Turbo_createPermutation( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );

void Ldpc_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Ldpc_get_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Ldpc_set_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Ldpc_set_encoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );

void Convolutional_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Convolutional_get_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Convolutional_set_decoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
void Convolutional_set_encoderOptions( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );

void Trellis_constructor( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );