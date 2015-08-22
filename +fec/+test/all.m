function success = all
    T = 32;
    trellis = poly2trellis(4, [13, 15], 13);
    turboTrellis = poly2trellis(4, [15], 13);
    code{1} = fec.ConvolutionalCode(trellis, T);
    code{2} = fec.TurboCode({turboTrellis, turboTrellis}, {randperm(T), randperm(T)}, fec.TrellisEndType.Truncation, 5, fec.SchedulingType.Parallel);
    code{3} = fec.LdpcCode(dvbs2ldpc(1/2));

    success = true;
    for i = 1:length(code)
        success = success && fec.test.decode(code{i});
        success = success && fec.test.softOutDecode(code{i});
        success = success && fec.test.appDecode(code{i});
        success = success && fec.test.saveload(code{i});
    end
end