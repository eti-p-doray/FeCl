function TurboCode
%This function shows how to construct a turbo code.

    %We define a trellis structure from a polynomial.
    %   This structure is recursive.
    %   We do not need to add the systematic part of the trellis since the
    %   turbo code implicitly use it.
    trellis = poly2trellis(4, [15], 13);
    
    %We define an interleaved sequences of indices
    %These sequences will be used as interleaver for the first and
    %   second constituents of the parallel concatenated code.
    % The interleavers are used such that the entire sequence of bits 
    %   is read out from a sequence of addresses that are
    %   defined by these indices array.
    % We can create a turbo code with its constituents of different sizes.
    %   The constituent size is define by the length of the interleaver.
    pi{1} = [1:256];
    pi{2} = randperm(256);
    
    %We define a simple code with the trellis and the interleavers.
    code = fec.TurboCode(trellis, pi);

    %The code is generalized to any number of constituents. Different trellis structures can be used.
    pi{3} = randperm(128);
    code = fec.TurboCode({trellis, trellis, trellis}, pi, {TrellisEndType.paddingTail, TrellisEndType.Truncation});
    
    %We can specify the number of iteration for decoding. default = 5
    code = fec.TurboCode({trellis, trellis}, {pi1, pi2}, TrellisEndType.paddingTail, 5);
  
    %We can specify the decoder scheduling type. 
    %In serial decoding,
    %   each constituent tries to decode and gives its extrinsic
    %   information to the next constituent in a serial behavior.
    %In parallel decoding, each constituent tries to decode in parallel.
    %   The extrinsic information is then combined and shared to every
    %   constituents.
    %   Serial | Parallel default = Serial
    code = fec.TurboCode({trellis, trellis}, {pi1, pi2}, TrellisEndType.paddingTail, 5, fec.StructureType.Parallel, fec.MapType.LogMap)
    
    %We can also specify the decoder algorithm.: 
    %   LogMap | MaxLogMap default = MaxLogMap
    code = fec.TurboCode({trellis, trellis}, {pi1, pi2}, TrellisEndType.paddingTail, 5, fec.StructureType.Parallel, fec.MapType.LogMap)
    
    %And as all codes, with can change the number of thread used for
    %operations. In this case, we are using 2 threads.
    code = fec.TurboCode({trellis, trellis}, {pi1, pi2}, TrellisEndType.paddingTail, 5, fec.StructureType.Parallel, fec.MapType.LogMap, 2)

    %Lets now use the code for some operations. Since all code offer the
    %same interface, we call the operation function which will work on any
    %defined code
    fec.example.operation(code);
    
end