function all
    N = 1024;
    T = 256;
    trellis = poly2trellis(4, [13, 15], 13);
    turboTrellis = poly2trellis(4, [15], 13);
    
    code{1} = fec.ConvolutionalCode(trellis, T);
    code{2} = fec.TurboCode({turboTrellis, turboTrellis}, {randperm(T), randperm(T)}, fec.TrellisEndType.PaddingTail, 5, fec.StructureType.Serial, fec.MapType.LogMap);
    %code{3} = fec.LdpcCode(dvbs2ldpc(1/2));
    
    pi = randperm(T);
    matlabEncoder{2} = comm.TurboEncoder('TrellisStructure', trellis, 'InterleaverIndices', pi);
    matlabDecoder{2} = comm.TurboDecoder('TrellisStructure', trellis, 'InterleaverIndices', pi, 'NumIterations', 5);
    
    codeDesc = {...
        'Convolutional code ',...
        'Turbo code',...
        'Ldpc code'
        };
    
    snrdb = [-4:2:4];
    
    [errorCount, elapsedTime] = fec.performance.matlabDecode(matlabEncoder{2}, matlabDecoder{2}, T, N, snrdb);
    
    snrdbCell = num2cell(snrdb);
    elapsedTimeCell = num2cell(elapsedTime);
    errorCountCell = num2cell(errorCount);
    {'snr values' snrdbCell{:}; 'Elapsed time is' elapsedTimeCell{:}; 'Error count is' errorCountCell{:}}

    success = true;
    for i = 1:length(code)
        disp(['For ' codeDesc{i} ' with ' int2str(code{i}.msgSize) ' msg bits and ' int2str(N) ' blocs']);
        [errorCount, elapsedTime] = fec.performance.decode(code{i}, N, snrdb);
        
        snrdbCell = num2cell(snrdb);
        elapsedTimeCell = num2cell(elapsedTime);
        errorCountCell = num2cell(errorCount);
        {'snr values' snrdbCell{:}; 'Elapsed time is' elapsedTimeCell{:}; 'Error count is' errorCountCell{:}}
    end
end