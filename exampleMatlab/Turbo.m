%% Turbo Codec object
% *Package:* fec
%
% Provide encoding and decoding methods following a generalized Turbo structure.

%% Description
% The Turbo class is a specialisation of the generic Codec class.
% Its structure is a generalization of Turbo
%
% <include>operations</include>
%

%% This function shows how to construct a simple turbo code.
% See 'operation' for encoding and decoding operations

%We define a trellis structure from a polynomial.
%   This structure is recursive.
%   We do not need to add the systematic part of the trellis since the
%   turbo code implicitly use it.
%trellis = poly2trellis(4, [15], 13);

%We define interleaved sequences of indices
%These sequences will be used as interleaver for the first and
%   second constituents of the parallel concatenated code.
% The interleavers are used such that the entire sequence of bits 
%   is read out from a sequence of addresses that are
%   defined by these indices array.
% The first constituent will be given systematic inputs.
% The second constituent will be given random permutation of the input sequence.
%interl{1} = [];
%interl{2} = randperm(256);

%We define a simple code with the trellis and the interleavers.
%codec = fec.Turbo(trellis, interl)

%Lets now use the code for some operations. Since all code offer the
%same interface, we call the operation function which will work on any
%defined code
%operations(codec);
%moreOperations(codec);