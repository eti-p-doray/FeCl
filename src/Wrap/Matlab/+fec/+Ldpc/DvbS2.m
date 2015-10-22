classdef DvbS2
    methods (Static)
        function mat = matrix(length, rate)
            mat = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_DvbS2_matrix), length, rate);
        end
    end
end