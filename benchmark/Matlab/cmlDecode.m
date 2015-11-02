function results = cmlDecode(sim, codec, msg, llr, N, z)
    [decodedMsg, ~] = CmlDecode(double(llr(:,1).'), double(msg(:,1).'), sim, codec);
    elapsedTime = zeros(N,1);
    errorCount = zeros(N,1);
    for i = 1:N
        tic;
        for j = 1:size(msg,2)
            [decodedMsg, error] = CmlDecode(double(llr(:,j).'), double(msg(:,j).'), sim, codec);
            errorCount(i) = errorCount(i) + error(end);
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
    results.per = errorCount;
end