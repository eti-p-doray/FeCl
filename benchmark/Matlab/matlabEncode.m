function results = matlabEncode(encoder, msg, N, z)
    encoder.step(msg(:,1));
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic
        for j = 1:size(msg,2)
            encoder.step(msg(:,j));
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
end