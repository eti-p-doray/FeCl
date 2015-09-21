classdef Structure < hgsetget
    properties
        encoderOptions;
        decoderOptions = fec.Convolutional.DecoderOptions();
    end
    properties(Dependent = true)
        trellis;
        length;
        termination;
        
        algorithm;
    end

    methods
        function self = Structure(varargin)
            if (isa(varargin{1}, 'fec.Convolutional.EncoderOptions') || (isfield(varargin{1}, 'trellis') && isfield(varargin{1}, 'length')) || iscell(varargin{1}))
                self.encoderOptions = fec.Convolutional.EncoderOptions(varargin{1});
                if (nargin > 1)
                    self.decoderOptions = fec.Convolutional.DecoderOptions(varargin{2});
                end
            else
                self.encoderOptions = fec.Convolutional.EncoderOptions(varargin{1}, varargin{2});
                if (~isempty({varargin{3:end}}))
                    self.set(varargin{3:end});
                end
            end
        end
        
        function val = getEncoderOptions(self)
            val = self.encoderOptions.get();
        end
        function val = getDecoderOptions(self)
            val = self.decoderOptions.get();
        end

        function self = set.trellis(self,val)
            self.encoderOptions.trellis = val;
        end
        function self = set.length(self,val)
            self.encoderOptions.length = val;
        end
        function self = set.termination(self,val)
            class(self.decoderOptions);
            self.encoderOptions.termination = val;
        end
        function self = set.algorithm(self,val)
            self.decoderOptions.algorithm = val;
        end
    end
end