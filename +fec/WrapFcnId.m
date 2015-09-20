classdef WrapFcnId < uint32
  enumeration
    Codec_destroy(0)
    Codec_save(1)
    Codec_load(2)
    Codec_encode(3)
    Codec_decode(4)
    Codec_soDecode(5)
    Codec_get_msgSize(6)
    Codec_get_systSize(7)
    Codec_get_stateSize(8)
    Codec_get_paritySize(9)
    Codec_get_workGroupSize(10)
    Codec_set_workGroupSize(11)
    Turbo_constructor(12)
    Turbo_get_decoderOptions(13)
    Turbo_set_decoderOptions(14)
    Turbo_set_encoderOptions(15)
    Ldpc_constructor(16)
    Ldpc_get_decoderOptions(17)
    Ldpc_set_decoderOptions(18)
    Ldpc_set_encoderOptions(19)
    Convolutional_constructor(20)
    Convolutional_get_decoderOptions(21)
    Convolutional_set_decoderOptions(22)
    Convolutional_set_encoderOptions(23)
  end
end