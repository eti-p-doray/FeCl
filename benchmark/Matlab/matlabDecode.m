function results = matlabDecode(decoder, msg, llr, N, z)
    decodedMsg = int8(decoder.step(llr(:,1)));
    elapsedTime = zeros(N,1);
    errorCount = zeros(N,1);
    for i = 1:N
        tic;
        for j = 1:size(msg,2)
            decodedMsg = int8(decoder.step(llr(:,j)));
            errorCount(i) = errorCount(i) + sum(sum((decodedMsg-msg(:,j))~=0));
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
end