function results = matlabEncode(encoder, msg, z)
    encoder.step(double(msg{1}(:,1)));
    elapsedTime = zeros(length(msg),1);
    for i = 1:length(msg)
        tic
        for j = 1:size(msg{i},2)
            encoder.step(double(msg{i}(:,j)));
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(msg));
end