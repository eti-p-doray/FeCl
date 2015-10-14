%This file shows how to construct a simple convolutional code with more advanced options.
%See 'operation' for encoding and decoding operations

%We define a trellis structure from a polynomial.
%   This structure is systematic and recursive.
trellis = poly2trellis(4, [13, 15], 13)

%We define a simple code with the trellis.
%We want the code to use blocs of size 256 bits.
codec = fec.Convolutional(trellis, 256)

%We can specify the trellis end type :
%   Tail | Truncate default = Tail
codec = fec.Convolutional(trellis, 256, 'termination', 'Tail')

%We can also specify the decoder algorithm (only use for appDecode):
%   Exact | Linear | Approximate default = Linear
codec = fec.Convolutional(trellis, 256, 'termination', 'Tail', 'algorithm', 'Approximate')

%And as all codecs, with can change the number of thread used for
%operations. In this case, we are using 2 threads.
codec.workGroupSize = 2;