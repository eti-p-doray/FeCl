function [avElapsedTime, stdElapsedTime] = fecEncode(code, msg, N)
    code.encode(msg);
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic
        code.encode(msg);
        elapsedTime(i) = toc;
    end
    avElapsedTime = mean(elapsedTime);
    stdElapsedTime = std(elapsedTime);
end