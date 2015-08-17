function [avElapsedTime, stdElapsedTime] = fecDecode(code, msg, llr, N)
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic;
        decodedMsg = int8(code.decode(llr));
        %errorCount = sum(sum((decodedMsg-msg)~=0));
        elapsedTime(i) = toc;
    end
    avElapsedTime = mean(elapsedTime);
    stdElapsedTime = std(elapsedTime);
end