%> This class contains functions associated with the DvbS2 standard.
classdef DvbS2
    methods (Static)
        %>  Access an ldpc matrix defined in the standard.
        %>  @param  length  Lenght of codeword.
        %>  @param  rate    Rate associated with the matrix.
        %>  @return A sparse matrix as defined in the standard.
        function mat = matrix(length, rate)
            mat = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_DvbS2_matrix), length, rate);
        end
    end
end