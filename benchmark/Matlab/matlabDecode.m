function results = matlabDecode(decoder, msg, llr, z)
    decodedMsg = decoder.step(llr{1}(:,1));
    elapsedTime = zeros(length(llr),1);
    errorCount = zeros(1,length(llr));
    blocErrorCount = zeros(1,length(llr));
    for i = 1:length(llr)
        tic;
        for j = 1:size(msg{i},2)
            decodedMsg = decoder.step(llr{i}(:,j));
            errorCount(i) = errorCount(i) + sum(decodedMsg~=msg{i}(:,j));
            blocErrorCount(i) = blocErrorCount(i) + (sum(decodedMsg~=msg{i}(:,j))~=0);
        end
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(llr));
    results.ber = errorCount / double(size(llr{1},2)*size(msg{1},1));
    results.wer = blocErrorCount / double(size(llr{1},2));
end