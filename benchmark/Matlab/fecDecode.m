function results = fecDecode(codec, msg, llr, z)
    decodedMsg = codec.decode(llr{1});
    elapsedTime = zeros(length(llr),1);
    errorCount = zeros(1,length(llr));
    blocErrorCount = zeros(1,length(llr));
    for i = 1:length(llr)
        tic;
        decodedMsg = codec.decode(llr{i});
        errorCount(i) = sum(sum(decodedMsg~=msg{i}));
        blocErrorCount(i) = sum((sum(decodedMsg~=msg{i})~=0));
        elapsedTime(i) = toc;
    end
    results.avg = mean(elapsedTime);
    results.intvl = std(elapsedTime) * z / sqrt(length(llr));
    results.ber = errorCount / double(size(llr{1},2)*codec.msgSize);
    results.wer = blocErrorCount / double(size(llr{1},2));
end