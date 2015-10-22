%> This class contains functions associated with the 3GPP LTE standard.
classdef Lte3Gpp
    methods (Static)
        %>  Access an interleaver defined in the standard.
        %>  @param  length  Lenght of the interleaver to be created.
        %>  @return An array of index of the specified length as defined in the standard.
        function perm = interleaver(length)
            perm = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Turbo_Lte3Gpp_interleaver), length);
        end
    end
end