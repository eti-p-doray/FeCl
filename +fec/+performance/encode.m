function elapsedTime = encode(code, msg)
    tic
    code.encode(msg);
    elapsedTime = toc;
end