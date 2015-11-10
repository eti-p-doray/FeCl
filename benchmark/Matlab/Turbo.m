function results = Turbo(snrdb, T, N, z)
    trellis = poly2trellis(4, [15, 13], 15);
    turboTrellis = poly2trellis(4, 13, 15);
    pi = randperm(T);
    
    codec{1} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Exact');
    codec{2} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Linear');
    codec{3} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Approximate');
    codec{4} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Approximate', 'scalingFactor', 0.7);
    
     matlabEncoder = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
     matlabDecoder{1} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'True APP', 'NumIterations', 4);
     matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max*', 'NumIterations', 4);
     matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max', 'NumIterations', 4);
     
     cmlSim{1}.rate = 1/3;
     cmlSim{1}.max_iterations = 4;
     cmlSim{1}.comment = 'Turbo';
     cmlSim{1}.sim_type = 'coded';
     cmlSim{1}.code_configuration = 1;
     cmlSim{1}.SNR = -5.0;
     cmlSim{1}.SNR_type = 'Eb/No in dB';
     cmlSim{1}.framesize = T;
     cmlSim{1}.modulation = 'BPSK';
     cmlSim{1}.mod_order = 2;
     cmlSim{1}.mapping = [];
     cmlSim{1}.channel = 'AWGN';
     cmlSim{1}.bicm = 0;
     cmlSim{1}.demod_type = 0;
     cmlSim{1}.linetype = 'k:';
     cmlSim{1}.legend = cmlSim{1}.comment;
     cmlSim{1}.g1 = [1 1 0 1; 1 0 1 1];
     cmlSim{1}.nsc_flag1 = 0;
     cmlSim{1}.pun_pattern1 = [];
     cmlSim{1}.tail_pattern1 = [];
     cmlSim{1}.g2 = [1 1 0 1; 1 0 1 1];
     cmlSim{1}.nsc_flag2 = 0;
     cmlSim{1}.pun_pattern2 = [];
     cmlSim{1}.tail_pattern2 = [];
     cmlSim{1}.code_interleaver = 'randperm(40)-1';
     cmlSim{1}.decoder_type = 4;
     cmlSim{1}.reset = 0;
     cmlSim{1}.max_trials = 1;
     cmlSim{1}.minBER = 1e-6;
     cmlSim{1}.max_frame_errors = 1;
     cmlSim{1}.save_rate = 50;
     cmlSim{1}.filename = strcat('', 'convR1by2K7AWGN.mat');
     
     [cmlSim{1}, cmlCodec{1}] = InitializeCodeParam( cmlSim{1}, pwd );
     cmlCodec{1}.code_interleaver = pi-1;
     cmlCodec{1}.data_bits_per_frame = T;
     cmlSim{2} = cmlSim{1};
     cmlSim{2}.decoder_type = 3;
     [cmlSim{2}, cmlCodec{2}] = InitializeCodeParam( cmlSim{2}, pwd );
     cmlCodec{2}.code_interleaver = pi-1;
     cmlCodec{2}.data_bits_per_frame = T;
     cmlSim{3} = cmlSim{1};
     cmlSim{3}.decoder_type = 1;
     [cmlSim{3}, cmlCodec{3}] = InitializeCodeParam( cmlSim{3}, pwd );
     cmlCodec{3}.code_interleaver = pi-1;
     cmlCodec{3}.data_bits_per_frame = T;
    
     perm = codec{1}.puncturing();
     punc = codec{1}.puncturing('mask', [1 1; 1 0; 1 0]);
 
     snr = 10.0.^(snrdb/10.0);
     for i = 1:length(snr)
        msg{i} = uint64(randi([0 1],codec{1}.msgSize,N));
        parity{i} = punc.permute(codec{1}.encode(msg{i}));
        symbol{i} = double( -2*double(parity{i})+1 );
        signal{i} = symbol{i} + randn(size(parity{i})) / sqrt(2*snr(i));
        llr{i} = -4.0 * punc.dePermute(signal{i} * snr(i));
        llrAlt{i} = perm.permute(llr{i});
     end
 
     codec{1}.workGroupSize = 1;
     results.encoding.fecl1 = fecEncode(codec{1}, msg, z);
     codec{1}.workGroupSize = 4;
     results.encoding.fecl4 = fecEncode(codec{1}, msg, z);
     
     results.encoding.cml = cmlEncode(cmlSim{1}, cmlCodec{1}, msg, z);
     
     results.encoding.matlab = matlabEncode(matlabEncoder, msg, z);
     
     config = {'Exact', 'Table', 'Approximate', 'ApproximateScaling'}; 
    
    for i = 1:3
        codec{i}.workGroupSize = 1;
        results.decoding.(config{i}).fecl1 = fecDecode(codec{i}, msg, llr, z);
        results.decoding.(config{i}).fecl1.snr = snrdb;
        codec{i}.workGroupSize = 4;
        results.decoding.(config{i}).fecl4 = fecDecode(codec{i}, msg, llr, z);
        results.decoding.(config{i}).fecl4.snr = snrdb;
        
        results.decoding.(config{i}).cml = cmlDecode(cmlSim{i}, cmlCodec{i}, msg, llrAlt, z);
        results.decoding.(config{i}).cml.snr = snrdb;

        results.decoding.(config{i}).matlab = matlabDecode(matlabDecoder{i}, msg, llrAlt, z);
        results.decoding.(config{i}).matlab.snr = snrdb;
    end
    codec{4}.workGroupSize = 1;
    results.decoding.(config{4}).fecl1 = fecDecode(codec{4}, msg, llr, z);
    results.decoding.(config{4}).fecl1.snr = snrdb;
    codec{4}.workGroupSize = 4;
    results.decoding.(config{4}).fecl4 = fecDecode(codec{4}, msg, llr, z);
    results.decoding.(config{4}).fecl1.snr = snrdb;
    
end