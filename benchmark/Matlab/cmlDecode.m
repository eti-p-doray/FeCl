function results = cmlDecode(sim, codec, msg, llr, N, z)
    [decodedMsg, ~] = CmlDecode(llr.', double(msg(:,1).'), sim, codec);
    elapsedTime = zeros(N,1);
    errorCount = zeros(N,1);
    for i = 1:N
        tic;
        for j = 1:size(msg,2)
            [decodedMsg, ~] = CmlDecode(llr.', double(msg(:,j).'), sim, codec);
            %errorCount(i) = errorCount(i) + sum(sum((decodedMsg-double(msg(:,j).'))~=0));
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
end