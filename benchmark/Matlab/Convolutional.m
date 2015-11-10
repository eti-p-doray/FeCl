function results = Convolutional(snrdb, T, N, z)
    trellis = poly2trellis(4, [15, 13], 15);
    
    codec = fec.Convolutional(trellis, T, 'termination', 'Truncate');
    matlabEncoder = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);

    cmlSim.rate = 1/2;
    cmlSim.max_iterations = 1;
    cmlSim.comment = 'Conv';
    cmlSim.sim_type = 'coded';
    cmlSim.code_configuration = 0;
    cmlSim.SNR = -5.0;
    cmlSim.SNR_type = 'Eb/No in dB';
    cmlSim.framesize = T;
    cmlSim.modulation = 'BPSK';
    cmlSim.mod_order = 2;
    cmlSim.mapping = [];
    cmlSim.channel = 'AWGN';
    cmlSim.bicm = 0;
    cmlSim.demod_type = 0;
    cmlSim.linetype = 'k:';
    cmlSim.legend = cmlSim.comment;
    cmlSim.g1 = [1 1 0 1; 1 0 1 1];
    cmlSim.nsc_flag1 = 0;
    cmlSim.pun_pattern1 = [1; 1];
    cmlSim.tail_pattern1 = [0 0 0; 0 0 0];
    cmlSim.decoder_type = 1;
    cmlSim.reset = 0;
    cmlSim.max_trials = 1;
    cmlSim.minBER = 1e-6;
    cmlSim.max_frame_errors = 1;
    cmlSim.save_rate = 50;
    cmlSim.filename = strcat('', 'convR1by2K7AWGN.mat');

    [cmlSim, cmlCodec] = InitializeCodeParam( cmlSim, pwd );

    snr = 10.0.^(snrdb/10.0);
     for i = 1:length(snr)
        msg{i} = uint64(randi([0 1],codec.msgSize,N));
        parity{i} = codec.encode(msg{i});
        symbol{i} = double( -2*double(parity{i})+1 );
        signal{i} = symbol{i} + randn(size(parity{i})) / sqrt(2*snr(i));
        llr{i} = -4.0 * signal{i} * snr(i);
        llrAlt{i} = -llr{i};
     end
   
    codec.workGroupSize = 1;
    results.encoding.fecl1 = fecEncode(codec, msg, z);
    codec.workGroupSize = 4;
    results.encoding.fecl4 = fecEncode(codec, msg, z);

    results.encoding.cml = cmlEncode(cmlSim, cmlCodec, msg, z);

    results.encoding.matlab = matlabEncode(matlabEncoder, msg, z);
    
    codec.workGroupSize = 1;
    results.decoding.fecl1 = fecDecode(codec, msg, llr, z);
    results.decoding.fecl1.snr = snrdb;
    codec.workGroupSize = 4;
    results.decoding.fecl4 = fecDecode(codec, msg, llr, z);
    results.decoding.fecl4.snr = snrdb;
    
    results.decoding.cml = cmlDecode(cmlSim, cmlCodec, msg, llr, z);
    results.decoding.cml.snr = snrdb;    
    
    results.decoding.matlab = matlabDecode(matlabDecoder, msg, llrAlt, z);
    results.decoding.matlab.snr = snrdb;

end