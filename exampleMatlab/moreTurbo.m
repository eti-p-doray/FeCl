%This file shows how to construct a turbo code with more advanced options.
%See operation for encoding and decoding operations

%We define a trellis structure from a polynomial.
%   This structure is recursive.
%   We do not need to add the systematic part of the trellis since the
%   turbo code implicitly use it.
%The code is generalized to any number of constituents. Different trellis structures can be used.
trellis{1} = poly2trellis(4, [15], 13);
trellis{2} = poly2trellis(4, [15 17], 13);
trellis{3} = poly2trellis([3, 4], [3; 15], [7, 13]);

%We define interleaved sequences of indices
%These sequences will be used as interleaver for the first and
%   second constituents of the parallel concatenated code.
% The interleavers are used such that the entire sequence of bits
%   is read out from a sequence of addresses that are
%   defined by these indices array.
% We can create a turbo code with its constituents of different sizes.
%   The constituent size is defined by the length of the interleaver.
interl{1} = randperm(248);
interl{2} = randperm(256);
interl{3} = randperm(192, 128);

%We can specify the trellis termination type :
%   Tail | Truncate default = Tail
% Constituents can have different terminations
term{1} = 'Tail';
term{2} = 'Truncate';
term{3} = 'Tail';

codec = fec.Turbo(trellis, interl, 'termination', term)

% We can specify the number of iteration for decoding. default = 5
% We are replacing the code object by a new one.
codec = fec.Turbo(trellis, interl, 'termination', term, 'iterations', 4);

% Or we can change it after creation
codec.iterationCount = 6;

%We can specify the decoder scheduling type.
%In serial decoding,
%   each constituent tries to decode and gives its extrinsic
%   information to the next constituent in a serial behavior.
%In parallel decoding, each constituent tries to decode in parallel.
%   The extrinsic information is then combined and shared to every
%   constituents.
%   Serial | Parallel default = Serial
codec = fec.Turbo(trellis, interl, 'termination', term, 'iterations', 4, 'scheduling', 'Parallel')

%We can also specify the decoder algorithm.:
%   Exact | Linear | Approximate default = Linear
code = fec.TurboCode(trellis, interl, 'termination', term, 'iterations', 4, 'scheduling', 'Parallel', 'algorithm' 'Approximate')

%And as all codecs, with can change the number of thread used for
%operations. In this case, we are using 2 threads.
codec.workGroupSize = 2;