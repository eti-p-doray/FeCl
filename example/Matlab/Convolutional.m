%This file shows how to construct a simple convolutional code.
%See 'operation' for encoding and decoding operations

%>  [Creating a simple Convolutional Codec]
% We define a trellis structure from a polynomial.
% This structure is systematic and recursive.
trellis = poly2trellis(4, [13, 15], 13)

% We define a simple code with the trellis.
% We want the code to use blocs of size 256 bits.
codec = fec.Convolutional(trellis, 256)
%>  [Creating a simple Convolutional Codec]

%>  [Encode]
% Lets define a random sequence of message bits.
% The number of row must be equal to the code msgSize.
% Each row is one bloc of data encoded independently from the others.
% We are defining 5 bloc of data.
msg = uint64( randi([0 1],codec.msgSize,5) );

% We can encode the msg to obtain a sequence of parity bits.
c = codec.encode(msg);

%>  [Encode]

% Now we modulate (with bpsk) the parity bits.
x =  -2*double(c)+1;

% And we add AWGN noise.
snrdb = 1.0;
snr = 10.0^(snrdb/10.0);
y = x + randn(size(x)) / sqrt(2*snr);

% And we compute L-values from signal.
llr = -4.0 * y * snr;

%>  [Decode]
% Now, we decode and hope for success.
msgDecoded = codec.decode(llr);

% Lets now count the errors in the decoded msg
errorCount = sum(sum(msgDecoded ~= msg))

%>  [Decode]