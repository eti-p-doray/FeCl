function [errorCount] = per(code, msg, llr)
    elapsedTime = 0;
    errorCount = zeros(size(llr));

    for i = 1:length(llr)
        tic;
        decodedMsg = int8(code.decode(llr{i}));
        errorCount(i) = sum(sum((decodedMsg-msg)~=0));
        elapsedTime = elapsedTime + toc;
    end
    elapsedTime = elapsedTime / length(llr);
end