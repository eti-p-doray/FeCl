function [errorCount, elapsedTime] = decode(code, N, snrdb)
    elapsedTime = zeros(size(snrdb));
    errorCount = zeros(size(snrdb));

    msg = int8(randi([0 1],code.msgSize,N));

    parity = int8(code.encode(msg));


    snr = 10.0.^(snrdb/10.0);
    for i = 1:length(snr)
        tic;
        symbol = double( -2*double(parity)+1 );
        signal = symbol + randn(size(symbol)) / sqrt(2*snr(i));
    
        %And we compute L-values from signal.
        llr = -4.0 * signal * snr(i);
   
        decodedMsg = int8(code.decode(llr));
        errorCount(i) = sum(sum(decodedMsg-msg) ~= 0);
        elapsedTime(i) = toc;
    end
end