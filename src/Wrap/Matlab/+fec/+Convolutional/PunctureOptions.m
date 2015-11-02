%>  This class gathers options affecting puncturing in Convolutional Codec.
classdef PunctureOptions < hgsetget
    properties
        %>  Mask for parity bits.
        %>  This array is a mask for parity bits. A mask is a vector in the form of a sequence of 0 and 1, where only ones are kept. The mask sequence is repeated until the end of the parity sequence.
        mask = [];
        %>  Mask for parity tail bits.
        tailMask = [];
    end

    methods
        function self = PunctureOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Convolutional.PunctureOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            else
                if (~isempty(varargin))
                    self.set(varargin{:});
                end
            end
        end
        function val = getEncoderOptions(self)
            val = self.encoderOptions.get();
        end
        function val = getDecoderOptions(self)
            val = self.decoderOptions.get();
        end
    end
end