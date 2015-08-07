function succes = test_appDecode(code)
    msg = int8(randi([0 1],code.msgSize,5));

    parity = int8(code.encode(msg));
    llr = 2*parity-1;
    extrinsic = zeros(code.extrinsicSize, size(msg,2));

    decodedMsg1 = code.softOutDecode(llr);
    [decodedMsg2, extrinsic] = code.appDecode(llr, extrinsic);
    [decodedMsg3, extrinsic] = code.appDecode(llr, extrinsic);

    succes = sum(sum(int8(decodedMsg2 > 0) - msg)) == 0;
    succes = succes & sum(sum(int8(decodedMsg3 > 0) - msg)) == 0;
    succes = succes & sum(sum(decodedMsg2 - decodedMsg1)) == 0;
end