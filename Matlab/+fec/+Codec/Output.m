classdef Output
    properties(SetAccess = private)
        syst = false;
        parity = false;
        state = false;
    end
    properties(Access = private)
        index = 0;
    end

    methods
        function self = Output(varargin)
            for i = 1:length(varargin)
                self.(varargin{i}) = true;
            end
        end
        function self = set.syst(val)
            index = index+1;
            self.syst = index;
        end
    end
end