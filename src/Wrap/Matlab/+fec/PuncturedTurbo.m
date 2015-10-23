classdef PuncturedTurbo < fec.Turbo
    properties (Dependent = true, Hidden)
        punctureOptions
    end
    properties (Dependent = true)

    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Turbo();
            self.reload(s);
        end
    end

    methods
        function self = PuncturedTurbo(varargin)
            if (nargin > 0)
              self.structure = fec.PuncturedTurbo.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.PuncturedTurbo_constructor), self.structure.getEncoderOptions, self.structure.getPunctureOptions, self.structure.getDecoderOptions);
            end
        end
        function set.punctureOptions(self,val)
            self.structure.punctureOptions = fec.Turbo.PunctureOptions(val);
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.PuncturedTurbo_set_punctureOptions), self, self.structure.punctureOptions.get());
        end

        function setPunctureOptions(self,varargin)
            punctureOptions = self.punctureOptions;
            punctureOptions.set(varargin{:});
            self.punctureOptions = punctureOptions;
        end
    end

end