function ConvolutionalCode
%This function shows how to construct a simple convolutional code.
%See 'operation' for encoding and decoding operations

    %We define a trellis structure from a polynomial.
    %   This structure is systematic and recursive.
    trellis = poly2trellis(4, [13, 15], 13)
    
    %We define a simple code with the trellis.
    %We want the code to use blocs of size 256 bits.
    code = fec.ConvolutionalCode(trellis, 256)

    %Lets now use the code for some operations. Since all code offer the
    %same interface, we call the operation function which will work on any
    %defined code
    operation(code);
end