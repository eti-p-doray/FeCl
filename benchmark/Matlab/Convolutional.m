function results = Convolutional(snrdb, T, N, M, z)
    trellis = poly2trellis(4, [17, 15], 17);
    
    codec = fec.Convolutional(trellis, T, 'termination', 'Truncate');
    matlabEncoder = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);

    msg = int8(randi([0 1],codec.msgSize,N));
    parity = int8(codec.encode(msg));
         
    snr = 10.0.^(snrdb/10.0);
    symbol = double( -2*double(parity)+1 );
    signal = symbol + randn(size(parity)) / sqrt(2*snr);
    llr = -4.0 * signal * snr;

    codec.workGroupSize = 1;
    results.encoding.fec1 = fecEncode(codec, msg, M, z);
    codec.workGroupSize = 4;
    results.encoding.fec4 = fecEncode(codec, msg, M, z);
    
    results.encoding.matlab = matlabEncode(matlabEncoder, msg, M, z);
    
    codec.workGroupSize = 1;
    results.decoding.fec1 = fecDecode(codec, msg, llr, M, z);
    codec.workGroupSize = 4;
    results.decoding.fec4 = fecDecode(codec, msg, llr, M, z);
        
    results.decoding.matlab = matlabDecode(matlabDecoder, msg, llr, M, z);

end