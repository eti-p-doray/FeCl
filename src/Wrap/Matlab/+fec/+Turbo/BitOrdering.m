classdef BitOrdering < uint32
    %>  Ordering of parity bits in Turbo
    %>  This defines the ordering of parity bits that are output from a Turbo permutation of a PuncturedTurbo Codec.
    enumeration
        %>  Systematic and parity bits are alternated
        Alternate(0)
        %>  Systematic bits are group together and parity bits from each constituents are grouped together.
        Group(1)
    end
end