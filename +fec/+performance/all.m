function all
    N = {1024, 16, 64, 4, 4};
    T = 1024;
    trellis = poly2trellis(4, [13, 15], 13);
    turboTrellis = poly2trellis(4, [15], 13);
    
    code{1} = fec.ConvolutionalCode(trellis, T);
    pi = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 5, fec.StructureType.Serial, fec.MapType.LogMap);
    code{3} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 5, fec.StructureType.Serial, fec.MapType.MaxLogMap);
    code{4} = fec.LdpcCode(H, 50, fec.BpType.TrueBp);
    code{5} = fec.LdpcCode(H, 50, fec.BpType.MinSumBp);
    
    matlabEncoder{1} = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder{1} = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);
    matlabEncoder{2} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'True APP', 'NumIterations', 5);
    matlabEncoder{3} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max', 'NumIterations', 5);
    matlabEncoder{4} = comm.LDPCEncoder(H);
    matlabDecoder{4} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied');
    matlabEncoder{5} = comm.LDPCEncoder(H);
    matlabDecoder{5} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied');
    
    codeDesc = {...
        'Convolutional code ',...
        'Turbo code',...
        'Turbo code with MaxLogMap approximation'...
        'Ldpc code'...
        'Ldpc code with MinSum approximation (matlab does not support the approximation, standard computation is used)'
        };
    
    snrdb = {[-2:1:2], [-5:0.5:-3], [-5:0.5:-3] [-4:0.5:-2] [-3:0.5:-1]};

    for i = 1:length(code)
         msg = int8(randi([0 1],code{i}.msgSize,N{i}));
         parity = int8(code{i}.encode(msg));
         
         snr = 10.0.^(snrdb{i}/10.0);
         llr = cell(length(snr),1);
         for j = 1:length(snr)
            symbol = double( -2*double(parity)+1 );
            signal = symbol + randn(size(parity)) / sqrt(2*snr(j));
            llr{j} = -4.0 * signal * snr(i);
         end
         
         
        disp(codeDesc{i});
        disp([' For ' int2str(code{i}.msgSize) ' msg bits and ' int2str(N{i}) ' blocs']);
        
        fecEncElapsedTime = fec.performance.encode(code{i}, msg);
        [fecErrorCount, fecDecElapsedTime] = fec.performance.decode(code{i}, msg, llr);
        
        if (~isa(code{i}, 'fec.TurboCode'))
            for j = 1:length(snr)
                llr{j} = -llr{j};
            end
        else
            for l = 1:length(snr)
                llrTmp = llr{l};
                llr{l} = zeros(size(llr{l}));
                for j = 1:size(msg,2)
                    for k = 1:T
                        llr{l}((k-1)*3+1,j) = llrTmp(k,j);
                        llr{l}((k-1)*3+2,j) = llrTmp(k+1*T+6,j);
                        llr{l}((k-1)*3+3,j) = llrTmp(k+2*T+9,j);
                    end
                    for k = 1:3
                        llr{l}((k-1)*2+1+(T)*3,j) = llrTmp(k+T,j);
                        llr{l}((k-1)*2+2+(T)*3,j) = llrTmp(k+2*T+6,j);
                    end
                    for k = 1:3
                        llr{l}((k-1)*2+7+(T)*3,j) = llrTmp(k+T+3,j);
                        llr{l}((k-1)*2+8+(T)*3,j) = llrTmp(k+3*T+9,j);
                    end
                end
            end
        end
        
        matlabEncElapsedTime = fec.performance.matlabEncode(matlabEncoder{i}, msg);
        [matlabErrorCount, matlabDecElapsedTime] = fec.performance.matlabDecode(matlabDecoder{i}, msg, llr);
    
        disp('Bit Error Count');
        disp(table(reshape(snrdb{i},[],1), fecErrorCount, matlabErrorCount, 'VariableNames', {'snrdb', 'fec', 'matlab'}));
        
        encGain = (matlabEncElapsedTime)/fecEncElapsedTime * 100.0;
        decGain = (matlabDecElapsedTime)/fecDecElapsedTime * 100.0;
        disp('Operation elapsed time');
        disp(table([fecEncElapsedTime;fecDecElapsedTime], [matlabEncElapsedTime;matlabDecElapsedTime], [encGain; decGain], 'VariableNames', {'fec', 'matlab', 'gain'}, 'RowNames', {'Encoding', 'Decoding'}));
    end
end