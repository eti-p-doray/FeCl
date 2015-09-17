function success = softOutDecode(code)
    msg = int8(randi([0 1],code.msgSize,5));

    parity = int8(code.encode(msg));
    llr = 2*parity-1;
    decodedMsg = code.softOutDecode(llr);
    success = sum(sum(int8(decodedMsg > 0) - msg)) == 0;
end