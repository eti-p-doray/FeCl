function speed
    N = 2;
    M = 2;
    T = 32400;
    trellis = poly2trellis(4, [17, 15], 17);
    turboTrellis = poly2trellis(4, [15], 17);
    
    code{1} = fec.Convolutional(trellis, T, 'termination', 'Truncate');
    pi = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Exact');
    code{3} = fec.Turbo(turboTrellis, {[], pi}, 'termination', 'Tail', 'iterations', 4, 'scheduling', 'Serial', 'algorithm', 'Approximate');
    code{4} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Exact');
    code{5} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Approximate');
    
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

        code{i}.workGroupSize = 1;
        [fecAvEncEt1(i), fecStdEncEt1(i)] = fecEncode(code{i}, msg, M);
        [fecAvDecEt1(i), fecStdDecEt1(i), ~] = fecDecode(code{i}, msg, llr, M);

        code{i}.workGroupSize = 4;
        [fecAvEncEt4(i), fecStdEncEt4(i)] = fecEncode(code{i}, msg, M);
        [fecAvDecEt4(i), fecStdDecEt4(i), ~] = fecDecode(code{i}, msg, llr, M);
        
        if (~isa(code{i}, 'fec.TurboCode'))
            llr = -llr;
        end
        
        [matlabAvEncEt(i), matlabStdEncEt(i)] = matlabEncode(matlabEncoder{i}, msg, M);
        [matlabAvDecEt(i), matlabStdDecEt(i), ~] = matlabDecode(matlabDecoder{i}, msg, llr, M);
    end
    z = 1.96;
    disp('Encode');
    disp(table(...
        matlabAvEncEt,matlabStdEncEt * z / sqrt(M),...
        fecAvEncEt1, fecStdEncEt1 * z / sqrt(M),...
        fecAvEncEt4, fecStdEncEt4 * z / sqrt(M),...
        'VariableNames', {'matlab', 'Minc', 'fec1','f1inc', 'fec4', 'f4inc'}, 'RowNames', codeDesc));
    
    disp('Decode');
    disp(table(...
        matlabAvDecEt,matlabStdDecEt * z / sqrt(M),...
        fecAvDecEt1, fecStdDecEt1 * z / sqrt(M),...
        fecAvDecEt4, fecStdDecEt4 * z / sqrt(M),...
        'VariableNames', {'matlab', 'Minc', 'fec1','f1inc', 'fec4', 'f4inc'}, 'RowNames', codeDesc));
end