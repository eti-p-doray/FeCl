%> This class holds the DecoderAlgorithm enumeration.
classdef DecoderAlgorithm < uint32
    %>  Type of decoder algorithm.
    %>  This defines the type of algorithm used in decoding to compute the check update.
    enumeration
        %> No approximation is used and the L-values are computed in logarithmic domain.
        Exact(0)
        %> A lookup table with linear interpolation is used to compute the correction term.
        Linear(1)
        %> The correction term is ignored.
        Approximate(2)
    end
end