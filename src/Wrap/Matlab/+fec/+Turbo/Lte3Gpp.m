classdef Lte3Gpp

    methods (Static)
        function perm = interleaver(length)
            perm = fec.Permutation(fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_Lte3Gpp_interleaver), length), length);
        end
    end
end