function results = Ldpc(snrdb, T, N, z)
    H = dvbs2ldpc(1/2);
    
    codec{1} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Exact');
    codec{2} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Linear');
    codec{3} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Approximate');
    codec{4} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Approximate', 'scalingFactor', 0.9);

    matlabEncoder = comm.LDPCEncoder(H);
    matlabDecoder = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 20);

    MINBER = 1e-5;
    effective_rate = '1/2';
    cmlSim{1}.rate = effective_rate;
    cmlSim{1}.comment = 'Ldpc';
    cmlSim{1}.sim_type = 'coded';
    cmlSim{1}.code_configuration = 2; % LDPC
    cmlSim{1}.SNR = -5.0;
    cmlSim{1}.SNR_type = 'Eb/No in dB';
    cmlSim{1}.framesize = 64800;
    cmlSim{1}.parity_check_matrix = strcat( 'InitializeDVBS2(', effective_rate , ',', int2str( cmlSim{1}.framesize ), ')' );
    cmlSim{1}.modulation = 'BPSK';
    cmlSim{1}.mod_order = 2;
    cmlSim{1}.channel = 'AWGN';
    cmlSim{1}.bicm = 0;
    cmlSim{1}.demod_type = 0;
    cmlSim{1}.linetype = 'b:';
    cmlSim{1}.legend = strcat( 'Long r=', ' ', effective_rate );
    cmlSim{1}.max_iterations = 20;
    cmlSim{1}.decoder_type = 0;
    cmlSim{1}.filename = strcat( '', 'DVBS2longRate1by2_BPSK.mat');
    cmlSim{1}.reset = 0;
    cmlSim{1}.max_trials = 1;
    cmlSim{1}.minBER = MINBER;
    cmlSim{1}.max_frame_errors = 1;
    cmlSim{1}.plot_iterations = cmlSim{1}.max_iterations;
    cmlSim{1}.save_rate = 5;

    [cmlSim{1}, cmlCodec{1}] = InitializeCodeParam( cmlSim{1}, pwd );
    cmlSim{2} = cmlSim{1};
    cmlSim{2}.decoder_type = 1;
    [cmlSim{2}, cmlCodec{2}] = InitializeCodeParam( cmlSim{2}, pwd );
    
    snr = 10.0.^(snrdb/10.0);
     for i = 1:length(snr)
        msg{i} = uint64(randi([0 1],codec{1}.msgSize,N));
        parity{i} = codec{1}.encode(msg{i});
        symbol{i} = double( -2*double(parity{i})+1 );
        signal{i} = symbol{i} + randn(size(parity{i})) / sqrt(2*snr(i));
        llr{i} = -4.0 * signal{i} * snr(i);
        llrAlt{i} = -llr{i};
     end

    codec{1}.workGroupSize = 1;
    results.encoding.fecl1 = fecEncode(codec{1}, msg, z);
    codec{1}.workGroupSize = 4;
    results.encoding.fecl4 = fecEncode(codec{1}, msg, z);

    results.encoding.cml = cmlEncode(cmlSim{1}, cmlCodec{1}, msg, z);

    results.encoding.matlab = matlabEncode(matlabEncoder, msg, z);
    
    config = {'Exact', 'Table', 'Approximate', 'ApproximateScaling'};
    for i = 1:4
        codec{i}.workGroupSize = 1;
        results.decoding.(config{i}).fecl1 = fecDecode(codec{i}, msg, llr, z);
        results.decoding.(config{i}).fecl1.snr = snrdb;
        
        codec{i}.workGroupSize = 4;
        results.decoding.(config{i}).fecl4 = fecDecode(codec{i}, msg, llr, z);
        results.decoding.(config{i}).fecl4.snr = snrdb;
    end

    cmlConfig = {'Exact', 'Approximate'};
    for i = 1:2
        results.decoding.(cmlConfig{i}).cml = cmlDecode(cmlSim{i}, cmlCodec{i}, msg, llr, z);
        results.decoding.(cmlConfig{i}).cml.snr = snrdb;
    end

    results.decoding.(config{1}).matlab = matlabDecode(matlabDecoder, msg, llrAlt, z);
    results.decoding.(config{1}).matlab.snr = snrdb;
end