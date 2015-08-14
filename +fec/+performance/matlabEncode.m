function elapsedTime = matlabEncode(encoder, msg)
    encoder.step(msg(:,1));
    tic
    for i = 1:size(msg,2)
        encoder.step(msg(:,i));
    end
    elapsedTime = toc;
end