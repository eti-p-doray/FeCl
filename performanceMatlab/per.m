function per
    N = 8;
    M = 1;
    T = 32400;
    trellis = poly2trellis(4, [15, 17], 15);
    turboTrellis = poly2trellis(4, [17], 15);
    
    code{1} = fec.ConvolutionalCode(trellis, T);
    pi = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 4, fec.StructureType.Serial, fec.MapType.LogMap);
    code{3} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 4, fec.StructureType.Serial, fec.MapType.MaxLogMap);
    code{4} = fec.LdpcCode(H, 20, fec.BpType.TrueBp);
    code{5} = fec.LdpcCode(H, 20, fec.BpType.MinSumBp);
    
    matlabEncoder{1} = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder{1} = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);
    matlabEncoder{2} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'True APP', 'NumIterations', 4);
    matlabEncoder{3} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max', 'NumIterations', 4);
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
    
    snrdb = {[-3:0.2:-1.0], [-3.0:0.1:-1.0], [-3.0:0.1:-1.0] [-3:0.1:-1.0] [-3:0.1:-1.0]};

    fecAvBer = cell(length(code), 1);
    matlabAvBer = cell(length(code), 1);

    for i = 1:length(code)
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

            [~, ~, fecBer(j)] = fec.performance.fecDecode(code{i}, msg, llr, M);
            
            if (~isa(code{i}, 'fec.TurboCode'))
                llr = -llr;
            else
                llrTmp = llr;
                llr = zeros(size(llr));
                for l = 1:size(msg,2)
                    for k = 1:T
                        llr((k-1)*3+1,l) = llrTmp(k,l);
                        llr((k-1)*3+2,l) = llrTmp(k+1*T+6,l);
                        llr((k-1)*3+3,l) = llrTmp(k+2*T+9,l);
                    end
                    for k = 1:3
                        llr((k-1)*2+1+(T)*3,l) = llrTmp(k+T,l);
                        llr((k-1)*2+2+(T)*3,l) = llrTmp(k+2*T+6,l);
                    end
                    for k = 1:3
                        llr((k-1)*2+7+(T)*3,l) = llrTmp(k+T+3,l);
                        llr((k-1)*2+8+(T)*3,l) = llrTmp(k+3*T+9,l);
                    end
                end
            end
            
            [~, ~, matlabBer(j)] = fec.performance.matlabDecode(matlabDecoder{i}, msg, llr, M);
        end
        [fecBer matlabBer]
        fecAvBer{i} = fecBer;
        matlabAvBer{i} = matlabBer;
    end
    save('per', 'fecAvBer', 'matlabAvBer', 'snrdb');
end