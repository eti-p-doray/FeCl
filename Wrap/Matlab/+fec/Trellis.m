%> Creates a trellis structure from polynomial generator
function trellis = Trellis(constraintLength, generator, feedback)
    trellis = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Trellis_constructor), oct2dec(constraintLength), oct2dec(generator), oct2dec(feedback));
end