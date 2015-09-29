function success = test_decode(codec)
    msg = int64(randi([0 1],codec.msgSize,5));

    parity = codec.encode(msg);
    llr = 2*double(parity)-1;
    msgDec = int64(codec.decode(llr));
    success = sum(sum((msgDec - msg)~=0)~=0) == 0;
end