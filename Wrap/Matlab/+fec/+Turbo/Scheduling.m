%> This class holds the Scheduling enumeration.
classdef Scheduling < uint32
    %>  Scheduling used in decoding.
    %>  This defines the scheduling of extrinsic communication between code constituents.
    enumeration
        %> Each constituent tries to decode and gives its extrinsic information to the next constituent in a serial behavior.
        Serial(0)
        %>  Each constituent tries to decode in parallel.
        %>  The extrinsic information is then combined and shared to every constituents similar to the Belief Propagation algorithm used in ldpc.
        Parallel(1)
        Custom(2)
    end
end