%This file shows how to construct a simple turbo codec.
%See 'operation' for encoding and decoding operations

%>  [Creating a simple Turbo Codec]
% To create a valid Turbo Codec, we need to give it at least a trellis and an interleaver.
% Optionally, we can configure several other parameters.

%>  [Creating a Trellis]
% We define a trellis structure with a constraint length of 4, one output of polynomial 15 and a feedback polynomial of 13.
% We do not need to add the systematic part for of the trellis for Turbo since it is implicitly used.
trellis = fec.Trellis(4, 15, 13);

% This function is compatible with the structure used in poly2trellis. Thus, the function is equivalent to the following
trellis = poly2trellis(4, 15, 13);
%>  [Creating a Trellis]

% We define interleaved sequences of indices
% These sequences will be used as interleaver for the first and second constituents of the parallel concatenated code.
% The interleavers are used such that the entire sequence of bits is read out from a sequence of addresses that are defined by these indices array.
% The first constituent will be given systematic inputs.
% The second constituent will be given random permutation of the input sequence.
interl{1} = [];
interl{2} = randperm(256);

%We define a simple codec with the trellis and the interleavers.
codec = fec.Turbo(trellis, interl)

%>  [Creating a simple Turbo Codec]