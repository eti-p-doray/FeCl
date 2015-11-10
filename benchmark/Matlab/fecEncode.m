function results = fecEncode(code, msg, z)
    code.encode(msg{1});
    elapsedTime = zeros(length(msg),1);
    for i = 1:length(msg)
        tic
        code.encode(msg{i});
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(msg));
end