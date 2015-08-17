function all
    N = 8;
    M = 8;
    T = 32400;
    trellis = poly2trellis(4, [13, 15], 13);
    turboTrellis = poly2trellis(4, [15], 13);
    
    code{1} = fec.ConvolutionalCode(trellis, T);
    pi = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 4, fec.StructureType.Serial, fec.MapType.LogMap);
    code{3} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.PaddingTail, 4, fec.StructureType.Serial, fec.MapType.MaxLogMap);
    code{4} = fec.LdpcCode(H, 10, fec.BpType.TrueBp);
    code{5} = fec.LdpcCode(H, 10, fec.BpType.MinSumBp);
    
    matlabEncoder{1} = comm.ConvolutionalEncoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated');
    matlabDecoder{1} = comm.ViterbiDecoder('TrellisStructure', trellis, 'TerminationMethod', 'Truncated', 'TracebackDepth', T);
    matlabEncoder{2} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'True APP', 'NumIterations', 5);
    matlabEncoder{3} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{3} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'Algorithm', 'Max', 'NumIterations', 5);
    matlabEncoder{4} = comm.LDPCEncoder(H);
    matlabDecoder{4} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 10);
    matlabEncoder{5} = comm.LDPCEncoder(H);
    matlabDecoder{5} = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 10);
    
    codeDesc = {...
        'Convolutional',...
        'Turbo',...
        'Turbo appr'...
        'Ldpc'...
        'Ldpc appr'
        };
    
    snrdb = -5.0;%{[-2:1:2], [-5:0.5:-3], [-5:0.5:-3] [-4:0.5:-2] [-3:0.5:-1]};

    matlabAvEncEt = zeros(length(code), 1);
    matlabStdEncEt = zeros(length(code), 1);
    matlabAvDecEt = zeros(length(code), 1);
    matlabStdDecEt = zeros(length(code), 1);
    fecAvEncEt1 = zeros(length(code), 1);
    fecStdEncEt1 = zeros(length(code), 1);
    fecAvEncEt4 = zeros(length(code), 1);
    fecStdEncEt4 = zeros(length(code), 1);
    fecAvDecEt1 = zeros(length(code), 1);
    fecStdDecEt1 = zeros(length(code), 1);
    fecAvDecEt4 = zeros(length(code), 1);
    fecStdDecEt4 = zeros(length(code), 1);

    for i = 1:length(code)
        disp(i)
         msg = int8(randi([0 1],code{i}.msgSize,N));
         parity = int8(code{i}.encode(msg));
         
         snr = 10.0.^(snrdb/10.0);
        symbol = double( -2*double(parity)+1 );
        signal = symbol + randn(size(parity)) / sqrt(2*snr);
        llr = -4.0 * signal * snr;

        %disp(codeDesc{i});
        %disp([' For ' int2str(code{i}.msgSize) ' msg bits and ' int2str(N{i}) ' blocs']);
        
        code{i}.workGroupSize = 1;
        [fecAvEncEt1(i), fecStdEncEt1(i)] = fec.performance.fecEncode(code{i}, msg, M);
        [fecAvDecEt1(i), fecStdDecEt1(i)] = fec.performance.fecDecode(code{i}, msg, llr, M);

        code{i}.workGroupSize = 4;
        [fecAvEncEt4(i), fecStdEncEt4(i)] = fec.performance.fecEncode(code{i}, msg, M);
        [fecAvDecEt4(i), fecStdDecEt4(i)] = fec.performance.fecDecode(code{i}, msg, llr, M);
        
        if (~isa(code{i}, 'fec.TurboCode'))
            llr = -llr;
        else
            llrTmp = llr;
            llr = zeros(size(llr));
            for j = 1:size(msg,2)
                for k = 1:T
                    llr((k-1)*3+1,j) = llrTmp(k,j);
                    llr((k-1)*3+2,j) = llrTmp(k+1*T+6,j);
                    llr((k-1)*3+3,j) = llrTmp(k+2*T+9,j);
                end
                for k = 1:3
                    llr((k-1)*2+1+(T)*3,j) = llrTmp(k+T,j);
                    llr((k-1)*2+2+(T)*3,j) = llrTmp(k+2*T+6,j);
                end
                for k = 1:3
                    llr((k-1)*2+7+(T)*3,j) = llrTmp(k+T+3,j);
                    llr((k-1)*2+8+(T)*3,j) = llrTmp(k+3*T+9,j);
                end
            end
        end
        
        [matlabAvEncEt(i), matlabStdEncEt(i)] = fec.performance.matlabEncode(matlabEncoder{i}, msg, M);
        [matlabAvDecEt(i), matlabStdDecEt(i)] = fec.performance.matlabDecode(matlabDecoder{i}, msg, llr, M); 

        %disp('Bit Error Count');
        %disp(table(reshape(snrdb{i},[],1), fecErrorCount, matlabErrorCount, 'VariableNames', {'snrdb', 'fec', 'matlab'}));
        
        %encGain = (matlabEncElapsedTime - fecEncElapsedTime)/matlabEncElapsedTime * 100.0;
        %decGain = (matlabDecElapsedTime - fecDecElapsedTime)/matlabDecElapsedTime * 100.0;
        %disp('Operation elapsed time');
        %disp(table([fecEncElapsedTime;fecDecElapsedTime], [matlabEncElapsedTime;matlabDecElapsedTime], 'VariableNames', {'fec', 'matlab', 'gain'}, 'RowNames', {'Encoding', 'Decoding'}));
    end
    disp(table(matlabAvEncEt, fecAvEncEt1, fecAvEncEt4, 'VariableNames', {'matlab', 'fec1', 'fec4'}, 'RowNames', codeDesc));
    disp(table(matlabAvDecEt, fecAvDecEt1, fecAvDecEt4, 'VariableNames', {'matlab', 'fec1', 'fec4'}, 'RowNames', codeDesc));
end