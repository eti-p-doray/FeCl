function succes = test_fec
    T = 1024;
    N = 1024;
    trellis = poly2trellis(4, [15], 13);
    code{1} = fec.ConvolutionalCode(trellis, T);
    code{2} = fec.TurboCode({trellis, trellis}, {randperm(T), randperm(T)});
    code{3} = fec.LdpcCode(dvbs2ldpc(1/2));

    snrdb = [-5.0 0.0 5.0];
    snr = log10(snrdb / 10.0);

    for i = 1:length(code)
        for j = 1:length(snr)
            performance_decode(code{i}, snr(j), N);
            performance_softOutDecode(code{i}, snr(j), N);
            performance_appDecode(code{i}, snr(j), N);
        end
    end
end