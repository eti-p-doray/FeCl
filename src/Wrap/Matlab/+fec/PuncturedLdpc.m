classdef PuncturedLdpc < fec.Ldpc
    properties (Dependent = true, Hidden)
        punctureOptions
    end
    properties (Dependent = true)

    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Ldpc();
            self.reload(s);
        end
    end

    methods
        function self = PuncturedLdpc(varargin)
            if (nargin > 0)
              self.structure = fec.PuncturedLdpc.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.PuncturedLdpc_constructor), self.structure.getEncoderOptions, self.structure.getPunctureOptions, self.structure.getDecoderOptions);
            end
        end
        function set.punctureOptions(self,val)
            self.structure.punctureOptions = fec.Ldpc.PunctureOptions(val);
            fec.bin.wrap(uint32(fec.WrapFcnId.PuncturedLdpc_set_punctureOptions), self, self.structure.punctureOptions.get());
        end

        function setEncoderOptions(self,varargin)
            encoderOptions = self.encoderOptions;
            encoderOptions.set(varargin{:});
            self.encoderOptions = encoderOptions;
            self.punctureOptions = self.punctureOptions;
        end
        function setPunctureOptions(self,varargin)
            punctureOptions = self.punctureOptions;
            punctureOptions.set(varargin{:});
            self.punctureOptions = punctureOptions;
        end
    end

end