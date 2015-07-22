function succes = test_fec
    T = 256;
    turboTrellis = poly2trellis(4, [15], 13);
    turboCode = fec.TurboCode({turboTrellis, turboTrellis}, {randperm(T), randperm(T)});

    succes = test_decode(turboCode, int8(randi([0 1],TurboCode.msgSize,N)));
end