function per
    N = 1;
    M = 1;
    T = 32400;
    trellis = poly2trellis(4, [15, 17], 15);
    turboTrellis = poly2trellis(4, [17], 15);
    
    code{1} = fec.Convolutional(trellis, T, 'termination', 'Truncate');
    interl = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.Turbo(turboTrellis, {[], interl}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Exact');
    code{3} = fec.Turbo(turboTrellis, {[], interl}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Approximate');
    code{4} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Exact');
    code{5} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Approximate');
    
    matlabEncoder{1} = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder{1} = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);
    matlabEncoder{2} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', interl);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', interl, 'Algorithm', 'True APP', 'NumIterations', 4);
    matlabEncoder{3} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', interl);
    matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', interl, 'Algorithm', 'Max', 'NumIterations', 4);
    matlabEncoder{4} = comm.LDPCEncoder(H);
    matlabDecoder{4} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 20);
    matlabEncoder{5} = comm.LDPCEncoder(H);
    matlabDecoder{5} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 20);
    
    
    parityIdxTmp = [T+1:3*T];
    parityIdx = {...
        [1:2*T], ...
        [1:T, parityIdxTmp(repmat(logical([1 0]), 1, T))], ...
        [1:T, parityIdxTmp(repmat(logical([1 0]), 1, T))], ...
        [1:2*T], ...
        [1:2*T], ...
    }
    
    snrdb = {[-3.0:0.2:-1.0], [-3.0:0.1:-1.0], [-3.0:0.1:-1.0] [-3.0:0.1:-1.0] [-3.0:0.1:-1.0]};

    fecAvBer = cell(length(code), 1);
    matlabAvBer = cell(length(code), 1);

    for i = 1:5
        disp(i)
        code{i}.workGroupSize = 4;
        
        fecBer = zeros(length(snrdb{i}),1);
        matlabBer = zeros(length(snrdb{i}),1);
        parfor j = 1:length(snrdb{i})
            msg = int8(randi([0 1],code{i}.msgSize,N));
            parity = int8(code{i}.encode(msg));

            snr = 10.0.^(snrdb{i}(j)/10.0);
            symbol = double( -2*double(parity(parityIdx{i},:))+1 );
            signal = zeros(size(parity));
            signal(parityIdx{i},:) = symbol + randn(size(symbol)) / sqrt(2*snr);
            llr = -4.0 * signal * snr;

            [~, ~, fecBer(j)] = fecDecode(code{i}, msg, llr, M);
            
            if (~isa(code{i}, 'fec.Turbo'))
                llr = -llr;
            end
            
            [~, ~, matlabBer(j)] = matlabDecode(matlabDecoder{i}, msg, llr, M);
        end
        [fecBer matlabBer]
        fecAvBer{i} = fecBer;
        matlabAvBer{i} = matlabBer;
    end
    save('per', 'fecAvBer', 'matlabAvBer', 'snrdb');
end