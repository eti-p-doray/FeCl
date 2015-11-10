%This file shows how to construct a simple ldpc codec.
%See 'operation' for encoding and decoding operations

%>  [Creating a simple Ldpc Codec]
%We define low density parity check matrixfrom the dvbs2 standard with a rate 1/2
H = fec.Ldpc.DvbS2.matrix(64800, 1/2);

% We define a simple code with the ldpc matrix.
% The codec constructor will check if the matrix is in a triangular shape (which is the case for dvbs2) and if not, will transforms it to allow inplace encoding.
codec = fec.Ldpc(H)
%>  [Creating a simple Ldpc Codec]
