function success = test_all
    T = 32;
    trellis = poly2trellis(4, [13, 15], 13);
    turboTrellis = poly2trellis(4, [15], 13);
    codec{1} = fec.Convolutional(trellis, T);
    codec{2} = fec.Turbo(turboTrellis, {randperm(T), randperm(T)}, 'termination', 'Truncate', 'iterations', 5, 'scheduling', 'Parallel');
    codec{3} = fec.Ldpc(dvbs2ldpc(1/2));

    success = true;
    for i = 1:length(codec)
        success = success && test_decode(codec{i});
        success = success && test_soDecode(codec{i});
        success = success && test_saveload(codec{i});
    end
end