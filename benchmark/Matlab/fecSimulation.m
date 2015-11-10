function result = fecSimulation(codec, parityIdx, N, M, snrdb)
    msg = logical( randi([0 1],codec.msgSize,N) );
    
    size(parityIdx.sequence);
    
    parity = codec.encode(msg);
    snr = 10.0.^(snrdb/10.0);
    errorCount = zeros(size(snr));
    blocErrorCount = zeros(size(snr));
    for i = 1:length(snr)
        for j = 1:M
            symbol = parityIdx.permute(-2*double(parity)+1);
            signal = symbol + randn(size(symbol)) / sqrt(2*snr(i));
            llr = parityIdx.dePermute(-4.0 * signal * snr(i));
            decodedMsg = codec.decode(llr);
            errorCount(i) = errorCount(i) + sum(sum(decodedMsg~=msg));
            blocErrorCount(i) = blocErrorCount(i) + sum(sum(decodedMsg~=msg)~=0);
        end
    end
    result.snrdb = snrdb;
    result.ber = errorCount / double(N*M*codec.msgSize);
    result.wer = blocErrorCount / double(N*M);
end