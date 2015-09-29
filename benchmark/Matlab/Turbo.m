function results = Turbo(snrdb, T, N, M, z)
    trellis = poly2trellis(4, [17, 15], 17);
    turboTrellis = poly2trellis(4, [15], 17);
    pi = randperm(T);
    
    codec{1} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Exact');
    codec{2} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Linear');
    codec{3} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Approximate');

    matlabEncoder = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{1} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'True APP', 'NumIterations', 4);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max*', 'NumIterations', 4);
    matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max', 'NumIterations', 4);
    
    
    [cmlSim{1}, cmlCodec{1}] = InitializeCodeParam( cmlSim{1}, pwd );
    cmlSim{2} = cmlSim{1};
    cmlSim{2}.decoder_type = 1;
    [cmlSim{2}, cmlCodec{2}] = InitializeCodeParam( cmlSim{2}, pwd );
    
    msg = int8(randi([0 1],codec{1}.msgSize,N));
    parity = int8(codec{1}.encode(msg));
    pi = codec(codec{1}.createPermutation());

    snr = 10.0.^(snrdb/10.0);
    symbol = double( -2*double(parity)+1 );
    signal = symbol + randn(size(parity)) / sqrt(2*snr);
    llr = -4.0 * signal * snr;

    codec{1}.workGroupSize = 1;
    results.encoding.fec1 = fecEncode(codec{1}, msg, M, z);
    codec{1}.workGroupSize = 4;
    results.encoding.fec4 = fecEncode(codec{1}, msg, M, z);
    
    results.encoding.matlab = matlabEncode(matlabEncoder, msg, M, z);
    
    config = {'Exact', 'Table', 'Approximate'};
    
    for i = 1:3
        codec{i}.workGroupSize = 1;
        results.decoding.(config{i}).fec1 = fecDecode(codec{i}, msg, llr, M, z);
        codec{i}.workGroupSize = 4;
        results.decoding.(config{i}).fec4 = fecDecode(codec{i}, msg, llr, M, z);

        results.decoding.(config{i}).matlab = matlabDecode(matlabDecoder{i}, msg, pi.permute(llr), M, z);
    end

end