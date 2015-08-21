function moreOperations(code)
%This function shows how to use a code to make operations.

    %Lets define a random sequence of message bits.
    %   The number of row must be equal to the code msgSize.
    %   Each row is one bloc of data encoded independently from the others.
    %   We are defining 5 bloc of data.
    msg = int8( randi([0 1],code.msgSize,5) );

    % This is quite intuitive.
    parity = code.encode(msg);

    %Now we modulate (with bpsk) the parity bits.
    symbol = double( -2*double(parity)+1 );

    %And we add noise.
    snrdb = 0.0;
    snr = 10.0^(snrdb/10.0);
    signal = symbol + randn(size(symbol)) / sqrt(2*snr);

    %And we compute L-values from signal.
    llr = -4.0 * signal * snr;

    %Now, we decode and hope for success.
    msgDecoded = code.decode(llr);

    %We can also get the a posteriori values.
    aPosteriori = code.softOutDecode(llr);

    %And we can use a priori knowledge about the code state.
    %This also gives us new extrinsic knowledge.
    %Here, we are actually giving a sequence of zero as extrinsic
    %information. This will yield the exact same result as softOutDecode.
    [aPosteriori, extrinsic] = code.appDecode(llr, zeros(code.extrinsicSize,5));

    %Lets now count the errors in the decoded msg
    errorCount = sum(sum((((aPosteriori)>0)-double(msg))~=0) ~= 0)
end