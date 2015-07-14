function bitErrorRate = ErrorCorrectingCodeDemo
    blocCount = 16;
    trellis = poly2trellis(4,[13,15],13);
    %pi = randperm(blocSize);
    %code = TurboCode(trellis, trellis, pi);
    %code = ConvolutionalCode(trellis, blocSize);
    
    % dvbs2ldpc(1/2)
    %tic
    code = LdpcCode(dvbs2ldpc(1/2));
    %toc

    msg = randi([0 1],code.messageSize,blocCount);
    
    tic
    parity = code.encode(msg);
    toc

    snrLog = -1.0;
    snr = 10.0.^(snrLog/10.0);
    parityExtrinsic = -((-2*double(parity)+1)*4.0*snr + randn(size(parity)) * sqrt(8.0*snr));
    
    tic
    msgDecoded1 = code.decode(parityExtrinsic);
    toc
    
    bitErrorRate = length(find(double(msgDecoded1)-msg));
    
    
end