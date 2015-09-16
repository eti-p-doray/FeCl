classdef Convolutional < fec.Codec
    properties (Dependent = true, Hidden)
        encoderOptions
        decoderOptions
    end
    properties (Dependent = true)
        algorithm;
    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Convolutional();
            self.reload(s);
        end
    end

    methods
        function self = Convolutional(varargin)
            if (nargin > 0)
              structure = fec.Convolutional.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Convolutional_constructor), structure.getEncoderOptions, structure.getDecoderOptions);
            end
        end
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
        end
        function val = get.decoderOptions(self)
            val = fec.Convolutional.DecoderOptions(fec.bin.wrap(uint32(fec.WrapFcnId.Convolutional_get_decoderOptions), self));
        end
        function set.decoderOptions(self,val)
            decoderOptions = fec.Convolutional.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Convolutional_set_decoderOptions), self, decoderOptions.get());
        end
        function set.encoderOptions(self,val)
            encoderOptions = fec.Convolutional.EncoderOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.Convolutional_set_encoderOptions), self, encoderOptions.get());
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