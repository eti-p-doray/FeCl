function [errorCount, elapsedTime] = matlabDecode(encoder, decoder, T, N, snrdb)
    elapsedTime = zeros(size(snrdb));
    errorCount = zeros(size(snrdb));

    msg = int8(randi([0 1],T,1));

    parity = int8(encoder.step(msg));


    snr = 10.0.^(snrdb/10.0);
    for i = 1:length(snr)
        tic;
        for j = 1:N
            symbol = double( -2*double(parity)+1 );
            signal = symbol + randn(size(symbol)) / sqrt(2*snr(i));

            %And we compute L-values from signal.
            llr = -4.0 * signal * snr(i);

            decodedMsg = int8(decoder.step(llr));
            errorCount(i) = errorCount(i) + (sum(decodedMsg-msg) ~= 0);
        end
        elapsedTime(i) = toc;
    end
end