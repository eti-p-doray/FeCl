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
              self.structure = fec.Turbo.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_constructor), self.structure.getEncoderOptions, self.structure.getDecoderOptions);
            end
        end
        function perms = createPermutation(self, options)
            if (nargin < 2)
                options = {};
            end
            options = fec.Turbo.PermuteOptions(options);
            perms = fec.Permutation(fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_createPermutation), self, options.get()), self.paritySize);
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
            val = self.structure.decoderOptions;
        end
        function val = get.encoderOptions(self)
            val = self.structure.encoderOptions;
        end
        function set.decoderOptions(self,val)
            self.structure.decoderOptions = fec.Turbo.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_set_decoderOptions), self, self.structure.decoderOptions.get());
        end
        function set.encoderOptions(self,val)
            self.structure.encoderOptions = fec.Turbo.EncoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Turbo_set_encoderOptions), self, self.structure.encoderOptions.get());
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