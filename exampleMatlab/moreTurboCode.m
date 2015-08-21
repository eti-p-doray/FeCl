function moreTurboCode
%This function shows how to construct a turbo code with more advanced options.
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
    %   PaddingTail | Truncation default = Truncation
    % Constituents can have different terminations
    term{1} = fec.TrellisEndType.PaddingTail;
    term{2} = fec.TrellisEndType.PaddingTail;
    term{3} = fec.TrellisEndType.PaddingTail;

    code = fec.TurboCode(trellis, interl, term)

    % We can specify the number of iteration for decoding. default = 5
    % We are replacing the code object by a new one.
    code = fec.TurboCode(trellis, interl, term, 4);

    % Or we can change it after creation
    code.iterationCount = 4;

    %We can specify the decoder scheduling type.
    %In serial decoding,
    %   each constituent tries to decode and gives its extrinsic
    %   information to the next constituent in a serial behavior.
    %In parallel decoding, each constituent tries to decode in parallel.
    %   The extrinsic information is then combined and shared to every
    %   constituents.
    %   Serial | Parallel default = Serial
    code = fec.TurboCode(trellis, interl, term, 5, fec.SchedulingType.Parallel, fec.MapType.LogMap)

    %We can also specify the decoder algorithm.:
    %   LogMap | MaxLogMap default = MaxLogMap
    code = fec.TurboCode(trellis, interl, term, 5, fec.SchedulingType.Parallel, fec.MapType.LogMap)

    %And as all codes, with can change the number of thread used for
    %operations. In this case, we are using 2 threads.
    code = fec.TurboCode(trellis, interl, term, 5, fec.SchedulingType.Parallel, fec.MapType.LogMap, 2)

    %Lets now use the code for some operations. Since all code offer the
    %same interface, we call the operation function which will work on any
    %defined code
    operations(code);

end