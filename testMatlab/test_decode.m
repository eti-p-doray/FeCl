function success = test_decode(codec)
    msg = randi([0 1],codec.msgSize,5);

    parity = codec.encode(msg);
    llr = 2*double(parity)-1;
    msgDec = codec.decode(llr);
    success = sum(sum((msgDec - msg)~=0)~=0) == 0;
end