function elapsedTime = performance_decode(code, snr, N)
    msg = int8(randi([0 1],code{i}.msgSize,N));

    parity = int8(code.encode(msg));
    llr = (2*parity-1) * sqrt(2*snr) + randn(size(parity));

    tic;
    decodedMsg = code.decode(llr);
    elapsedTime = toc;

end