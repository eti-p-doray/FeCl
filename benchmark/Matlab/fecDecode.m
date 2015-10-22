function results = fecDecode(code, msg, llr, N, z)
    decodedMsg = int8(code.decode(llr));
    elapsedTime = zeros(N,1);
    errorCount = zeros(N,1);
    for i = 1:N
        tic;
        decodedMsg = int8(code.decode(llr));
        errorCount(i) = sum(sum(decodedMsg~=msg));
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
    results.per = errorCount;
end