%This file shows how to construct a simple ldpc codec.
%See 'operation' for encoding and decoding operations

%>  [Creating a simple Ldpc Codec]
%We define low density parity check matrix from the dvbs2 standard with a rate 1/2
H = fec.Ldpc.DvbS2.matrix(64800, 1/2);

% We define a simple code with the ldpc matrix.
% The codec constructor will check if the matrix is in a triangular shape (which is the case for dvbs2) and if not, will transforms it to allow inplace encoding.
codec = fec.Ldpc(H)
%>  [Creating a simple Ldpc Codec]

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