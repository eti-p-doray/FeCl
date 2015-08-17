function [avElapsedTime, stdElapsedTime] = matlabEncode(encoder, msg, N)
    encoder.step(msg(:,1));
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic
        for j = 1:size(msg,2)
            encoder.step(msg(:,j));
        end
        elapsedTime(i) = toc;
    end
    
    avElapsedTime = mean(elapsedTime);
    stdElapsedTime = std(elapsedTime);
end