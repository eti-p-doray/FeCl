function success = decode(code)
    msg = int8(randi([0 1],code.msgSize,5));

    parity = int8(code.encode(msg));
    llr = 2*parity-1;
    decodedMsg = int8(code.decode(llr));
    success = sum(sum(decodedMsg - msg)) == 0;
end