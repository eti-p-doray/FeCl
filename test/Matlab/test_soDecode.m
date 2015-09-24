function success = test_soDecode(codec)
    msg = int8(randi([0 1],codec.msgSize,5));

    parity = int8(codec.encode(msg));
    llr = 2*parity-1;
    msgDec = codec.soDecode(llr);
    success = sum(sum((int8(msgDec > 0) - msg)~=0)~=0) == 0;
end