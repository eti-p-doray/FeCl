function results = cmlEncode(sim, codec, msg, z)
    CmlEncode(double(msg{1}(:,1).'), sim, codec);
    elapsedTime = zeros(length(msg),1);
    for i = 1:length(msg)
        tic
        for j = 1:size(msg{i},2)
            CmlEncode(double(msg{i}(:,j).'), sim, codec);
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(msg));
end