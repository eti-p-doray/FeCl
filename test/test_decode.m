function succes = test_decode(code, msg)
    parity = int8(code.encode(msg));
    llr = 2*parity-1;
    decodedMsg = int8(code.decode(llr));
    succes = sum(sum(decodedMsg - msg)) == 0;
end