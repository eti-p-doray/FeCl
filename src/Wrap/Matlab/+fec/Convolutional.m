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
              self.structure = fec.Convolutional.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_constructor), self.structure.getEncoderOptions, self.structure.getDecoderOptions);
            end
        end
        function perms = puncturing(self, varargin)
            options = fec.Convolutional.PunctureOptions(varargin{:});
            perms = fec.Permutation(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_puncturing), self, options.get()), self.paritySize);
        end
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
        end
        function val = get.decoderOptions(self)
            val = self.structure.decoderOptions;
        end
        function val = get.encoderOptions(self)
            val = self.structure.encoderOptions;
        end
        function set.decoderOptions(self,val)
            self.structure.decoderOptions = fec.Convolutional.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_set_decoderOptions), self, decoderOptions.get());
        end
        %function set.encoderOptions(self,val)
        %    self.structure.encoderOptions = fec.Convolutional.EncoderOptions(val);
        %    fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_set_encoderOptions), self, encoderOptions.get());
        %end
        function setDecoderOptions(self,varargin)
            decoderOptions = self.decoderOptions;
            decoderOptions.set(varargin{:});
            self.decoderOptions = decoderOptions;
        end
        %function setEncoderOptions(self,varargin)
        %    encoderOptions = self.encoderOptions;
        %    encoderOptions.set(varargin{:});
        %    self.encoderOptions = encoderOptions;
        %end
    end

end