classdef PunctureOptions < hgsetget
    properties
        mask = [];
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