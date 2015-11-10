%This file shows how to construct an ldpc code with more advanced options.
%See 'operation' for encoding and decoding operations

% We define low density parity check matrix from the dvbs2 standard with a rate 1/2
H = dvbs2ldpc(1/2);

% We define a simple code with the ldpc matrix.
% The codec constructor will check if the matrix is in a triangular shape (which is the case for dvbs2) and if not, will transforms it to allow inplace encoding.
codec = fec.Ldpc(H)

%We can specify the number of iteration for decoding. default = 50
% If a consistent msg is found before the maximum number of iteration,
% The decoder will stop and the number of iteration is not reached.
codec = fec.Ldpc(H, 'iterations', 10)

%We can also specify the decoder algorithm :
%   Exact | Linear | Approximate default = Linear
codec = fec.Ldpc(H, 'iterations', 10, 'algorithm', 'Approximate')

%And as all codecs, with can change the number of thread used for
%operations. In this case, we are using 2 threads.
codec.workGroupSize = 2;