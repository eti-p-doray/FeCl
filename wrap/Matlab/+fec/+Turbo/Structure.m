classdef Structure < hgsetget
    properties
        encoderOptions;
        decoderOptions = fec.Turbo.DecoderOptions();
    end
    properties(Dependent = true)
        trellis;
        interleaver;
        termination;
        
        iterations;
        algorithm;
        scheduling;
        scalingFactor;
    end

    methods
        function self = Structure(varargin)
            if (nargin > 0)
                if (isa(varargin{1}, 'fec.Turbo.EncoderOptions') || (isfield(varargin{1}, 'trellis') && isfield(varargin{1}, 'interleaver')))
                    self.encoderOptions = fec.Turbo.EncoderOptions(varargin{1});
                    if (nargin > 1)
                        self.decoderOptions = fec.Turbo.DecoderOptions(varargin{2});
                    end
                else
                    self.encoderOptions = fec.Turbo.EncoderOptions(varargin{1}, varargin{2});
                    if (~isempty(varargin(3:end)))
                        self.set(varargin{3:end});
                    end
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
        function self = set.interleaver(self,val)
            self.encoderOptions.interleaver = val;
        end
        function self = set.termination(self,val)
            class(self.decoderOptions);
            self.encoderOptions.termination = val;
        end
        
        function self = set.iterations(self,val)
            self.decoderOptions.iterations = val;
        end
        function self = set.algorithm(self,val)
            self.decoderOptions.algorithm = val;
        end
        function self = set.scheduling(self,val)
            self.decoderOptions.scheduling = val;
        end
        function self = set.scalingFactor(self,val)
            self.decoderOptions.scalingFactor = val;
        end
    end
end