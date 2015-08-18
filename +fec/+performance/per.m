function per
    N = 256;
    M = 8;
    T = 32400;
    trellis = poly2trellis(4, [15, 17], 15);
    turboTrellis = poly2trellis(4, [17], 15);
    
    code{1} = fec.ConvolutionalCode(trellis, T);
    pi = randperm(T);
    H = dvbs2ldpc(1/2);
    code{2} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.Truncation, 4, fec.StructureType.Serial, fec.MapType.LogMap);
    code{3} = fec.TurboCode(turboTrellis, {[1:T], pi}, fec.TrellisEndType.Truncation, 4, fec.StructureType.Serial, fec.MapType.MaxLogMap);
    code{4} = fec.LdpcCode(H, 20, fec.BpType.TrueBp);
    code{5} = fec.LdpcCode(H, 20, fec.BpType.MinSumBp);
    
    parityIdxTmp = [T+1:3*T];
    parityIdx = {...
        [1:2*T], ...
        [1:T, parityIdxTmp(repmat(logical([1 0]), 1, T))], ...
        [1:T, parityIdxTmp(repmat(logical([1 0]), 1, T))], ...
        [1:2*T], ...
        [1:2*T], ...
    }
    
    snrdb = {[-3:0.5:-1.0], [-3.0:0.2:0.0], [-3.0:0.2:0.0] [-3:0.2:0] [-3:0.2:0]};

    fecAvBer = cell(length(code), 1);

    for i = 1:length(code)
        disp(i)
        code{i}.workGroupSize = 4;
        
        ber = zeros(length(snrdb{i}),1);
        parfor j = 1:length(snrdb{i})
            msg = int8(randi([0 1],code{i}.msgSize,N));
            parity = int8(code{i}.encode(msg));

            snr = 10.0.^(snrdb{i}(j)/10.0);
            symbol = double( -2*double(parity(parityIdx{i},:))+1 );
            signal = zeros(size(parity));
            signal(parityIdx{i},:) = symbol + randn(size(symbol)) / sqrt(2*snr);
            llr = -4.0 * signal * snr;

            [~, ~, ber(j)] = fec.performance.fecDecode(code{i}, msg, llr, M);
        end
        fecAvBer{i} = ber;
    end
    save('per', 'fecAvBer', 'snrdb');
end