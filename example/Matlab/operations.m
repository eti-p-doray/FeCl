%This file shows how to use a code to make operations.

%Given a codec object, we will used the codec to exectue operations

%Lets define a random sequence of message bits.
%   The number of row must be equal to the code msgSize.
%   Each row is one bloc of data encoded independently from the others.
%   We are defining 5 bloc of data.
msg = logical( randi([0 1],codec.msgSize,5) );

% This is quite intuitive.
parity = codec.encode(msg);

%Now we modulate (with bpsk) the parity bits.
symbol =  -2*double(parity)+1;

%And we add noise.
snrdb = 0.0;
snr = 10.0^(snrdb/10.0);
signal = symbol + randn(size(symbol)) / sqrt(2*snr);

%And we compute L-values from signal.
llr = -4.0 * signal * snr;

%Now, we decode and hope for success.
msgDecoded = codec.decode(llr);

%Lets now count the errors in the decoded msg
errorCount = sum(sum((msgDecoded-msg)~=0) ~= 0)