function [errorCount, elapsedTime] = matlabDecode(decoder, msg, llr)
    elapsedTime = 0;
    errorCount = zeros(size(llr));
    
    decodedMsg = int8(decoder.step(llr{1}(:,1)));
    for i = 1:length(llr)
        tic;
        for j = 1:size(msg,2)
            decodedMsg = int8(decoder.step(llr{i}(:,j)));
            errorCount(i) = errorCount(i) + sum(sum((decodedMsg-msg(:,j))~=0));
        end
        elapsedTime = elapsedTime + toc;
    end
    elapsedTime = elapsedTime / length(llr);
end