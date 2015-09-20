classdef Input < matlab.mixin.SetGet
    properties
        syst;
        parity;
        state;
    end
    properties(Access = private)
        hasSyst = false;
        hasParity = false;
        hasState = false;
    end

    methods
        function self = Input(varargin)
            if (~isempty(varargin))
                self.set(varargin{:});
            end
        end

        function self = set.syst(self,val)
            self.syst = val;
            self.hasSyst = true;
        end
        function self = set.parity(self,val)
            self.parity = val;
            self.hasParity = true;
        end
        function self = set.state(self,val)
            self.state = val;
            self.hasState = true;
        end
    end
end