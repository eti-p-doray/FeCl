function succes = test_fec
    T = 32;
    N = 5;
    turboTrellis = poly2trellis(4, [15], 13);
    code{1} = fec.ConvolutionalCode(turboTrellis, T);
    code{2} = fec.TurboCode({turboTrellis, turboTrellis}, {randperm(T), randperm(T)});
    code{3} = fec.LdpcCode(dvbs2ldpc(1/2));

    succes = true;
    for i = 1:length(code)
        succes = succes && test_decode(code{i});
        succes = succes && test_softOutDecode(code{i});
        succes = succes && test_appDecode(code{i});
        succes = succes && test_saveload(code{i});
    end
end