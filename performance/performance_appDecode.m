function elapsedTime = test_appDecode(code, snr, N)
    msg = int8(randi([0 1],code{i}.msgSize,N));

    parity = int8(code.encode(msg));
    llr = (2*parity-1) * sqrt(2*snr) + randn(size(parity));
    extrinsic = zeros(code.extrinsicSize, size(msg,2)); 

    tic;
    [decodedMsg, extrinsic] = code.appDecode(llr, extrinsic);
    elapsedTime = toc;
end