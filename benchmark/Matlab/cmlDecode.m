function results = cmlDecode(sim, codec, msg, llr, z)
    [decodedMsg, ~] = CmlDecode(double(llr{1}(:,1).'), double(msg{1}(:,1).'), sim, codec);
    elapsedTime = zeros(length(llr),1);
    errorCount = zeros(1,length(llr));
    blocErrorCount = zeros(1,length(llr));
    for i = 1:length(msg)
        tic;
        for j = 1:size(msg{i},2)
            [decodedMsg, error] = CmlDecode(double(llr{i}(:,j).'), double(msg{i}(:,j).'), sim, codec);
            errorCount(i) = errorCount(i) + error(end);
            blocErrorCount(i) = blocErrorCount(i) + (error(end)~=0);
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(llr));
    results.ber = errorCount / double(size(llr{1},2)*size(msg{1},1));
    results.wer = blocErrorCount / double(size(llr{1},2));
end