function results = TurboScheduling(snrdb, T, N, z)
    turboTrellis = poly2trellis(4, 15, 13);
    pi{1} = [];
    pi{2} = randperm(T);
    pi{3} = randperm(T);
    
    codec{1} = fec.Turbo(turboTrellis, pi, 'termination', 'Truncate', 'iterations', 6, 'scheduling', 'Serial', 'algorithm', 'Linear');
    codec{2} = fec.Turbo(turboTrellis, pi, 'termination', 'Truncate', 'iterations', 6, 'scheduling', 'Parallel', 'algorithm', 'Linear');
    
    punc = codec{1}.puncturing('mask', [1 1 1; 1 0 0; 1 0 0; 1 0 0], 'bitOrdering', 'Group');
    snr = 10.0.^(snrdb/10.0);
     for i = 1:length(snr)
        msg{i} = (randi([0 1],codec{1}.msgSize,N));
        parity{i} = punc.permute(codec{1}.encode(msg{i}));
        symbol{i} = double( -2*double(parity{i})+1 );
        signal{i} = symbol{i} + randn(size(parity{i})) / sqrt(2*snr(i));
        llr{i} = -4.0 * punc.dePermute(signal{i} * snr(i));
     end
    
    config = {'Serial', 'Parallel'}; 
    for i = 1:2
        results.(config{i}) = fecDecode(codec{i}, msg, llr, z);
        results.(config{i}).snr = snrdb;
    end
end