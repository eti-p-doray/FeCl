classdef Turbo < fec.Codec
    properties (Dependent = true, Hidden)
        encoderOptions
        decoderOptions
    end
    properties (Dependent = true)
        iterations;
        algorithm;
        scheduling;
    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Turbo();
            self.reload(s);
        end
    end

    methods
        function self = Turbo(varargin)
            if (nargin > 0)
              structure = fec.Turbo.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_constructor), structure.getEncoderOptions, structure.getDecoderOptions);
            end
        end
        function val = get.iterations(self)
            val = self.decoderOptions.iterations;
        end
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        function val = get.scheduling(self)
            val = fec.Turbo.Scheduling(self.decoderOptions.scheduling).char;
        end
        function set.iterations(self, val)
            self.setDecoderOptions('iterations', val);
        end
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
        end
        function set.scheduling(self, val)
            self.setDecoderOptions('scheduling', val);
        end
        function val = get.decoderOptions(self)
            val = fec.Turbo.DecoderOptions(fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_get_decoderOptions), self));
        end
        function set.decoderOptions(self,val)
            decoderOptions = fec.Turbo.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_set_decoderOptions), self, decoderOptions.get());
        end
        function set.encoderOptions(self,val)
            encoderOptions = fec.Turbo.EncoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_set_encoderOptions), self, encoderOptions.get());
        end
        function setDecoderOptions(self,varargin)
            decoderOptions = self.decoderOptions;
            decoderOptions.set(varargin{:});
            self.decoderOptions = decoderOptions;
        end
        function setEncoderOptions(self,varargin)
            encoderOptions = self.encoderOptions;
            encoderOptions.set(varargin{:});
            self.encoderOptions = encoderOptions;
        end
    end

end