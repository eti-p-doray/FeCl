%> This class holds the Termination enumeration.
classdef Termination < uint32
    %>  Trellis termination types.
    %>  This specifies the type of termination at the end of each bloc.
    enumeration
        %>  The state is brought to zero by implicitly adding new msg bit.
        Tail(0)
        %>  The state is forced to zero by truncating the trellis.
        Truncate(1)
    end
end