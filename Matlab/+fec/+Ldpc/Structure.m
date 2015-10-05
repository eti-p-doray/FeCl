classdef Structure < hgsetget
    properties
        encoderOptions;
        decoderOptions = fec.Ldpc.DecoderOptions();
    end
    properties(Dependent = true)
        checkMatrix
        
        iterations;
        algorithm;
    end

    methods
        function self = Structure(varargin)
            if (isa(varargin{1}, 'fec.Ldpc.EncoderOptions') || (isfield(varargin{1}, 'checkMatrix')) || iscell(varargin{1}))
                self.encoderOptions = fec.Ldpc.EncoderOptions(varargin{1});
                if (nargin > 1)
                    self.decoderOptions = fec.Ldpc.DecoderOptions(varargin{2});
                end
            else
                self.encoderOptions = fec.Ldpc.EncoderOptions(varargin{1});
                if (~isempty({varargin{2:end}}))
                    self.set(varargin{2:end});
                end
            end
        end
        
        function val = getEncoderOptions(self)
            val = self.encoderOptions.get();
        end
        function val = getDecoderOptions(self)
            val = self.decoderOptions.get();
        end

        function self = set.checkMatrix(self,val)
            self.encoderOptions.checkMatrix = val;
        end
        
        function self = set.iterations(self,val)
            self.decoderOptions.iterations = val;
        end
        function self = set.algorithm(self,val)
            self.decoderOptions.algorithm = val;
        end
    end
end