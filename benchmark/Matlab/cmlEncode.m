function results = cmlEncode(sim, codec, msg, N, z)
    CmlEncode(double(msg(:,1).'), sim, codec);
    elapsedTime = zeros(N,1);
    for i = 1:N
        tic
        for j = 1:size(msg,2)
            CmlEncode(double(msg(:,j).'), sim, codec);
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(N);
end