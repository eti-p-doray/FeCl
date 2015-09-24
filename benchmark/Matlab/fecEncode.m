function results = fecEncode(code, msg, N, z)
    code.encode(msg);
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic
        code.encode(msg);
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
end