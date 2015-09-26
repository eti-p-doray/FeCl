function trellis = Trellis(constraintLength, generator, feedback)
  trellis = fec.bin.wrap(uint32(fec.WrapFcnId.Trellis_constructor), constraintLength, generator, feedback);
end