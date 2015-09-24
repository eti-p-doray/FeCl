function results = Ldpc(snrdb, T, N, M, z)
    H = dvbs2ldpc(1/2);
    
    codec{1} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Exact');
    codec{2} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Linear');
    codec{3} = fec.Ldpc(H, 'iterations', 20, 'algorithm', 'Approximate');

    matlabEncoder = comm.LDPCEncoder(H);
    matlabDecoder = comm.LDPCDecoder(H, 'IterationTerminationCondition', 'Parity check satisfied', 'MaximumIterationCount', 20);
    
    
    msg = int8(randi([0 1],codec{1}.msgSize,N));
    parity = int8(codec{1}.encode(msg));
         
    snr = 10.0.^(snrdb/10.0);
    symbol = double( -2*double(parity)+1 );
    signal = symbol + randn(size(parity)) / sqrt(2*snr);
    llr = -4.0 * signal * snr;

    codec{1}.workGroupSize = 1;
    results.encoding.fec1 = fecEncode(codec{1}, msg, M, z);
    codec{1}.workGroupSize = 4;
    results.encoding.fec4 = fecEncode(codec{1}, msg, M, z);
    
    results.encoding.matlab = matlabEncode(matlabEncoder, msg, M, z);
    
    config = {'Exact', 'Table', 'Approximate'};
    
    for i = 1:3
        codec{i}.workGroupSize = 1;
        results.decoding.(config{i}).fec1 = fecDecode(codec{i}, msg, llr, M, z);
        codec{i}.workGroupSize = 4;
        results.decoding.(config{i}).fec4 = fecDecode(codec{i}, msg, llr, M, z);
    end
    results.decoding.(config{1}).matlab = matlabDecode(matlabDecoder, msg, llr, M, z);
end