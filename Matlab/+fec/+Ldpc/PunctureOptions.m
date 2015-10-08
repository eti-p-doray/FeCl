classdef PunctureOptions < hgsetget
    properties
        mask;
        systMask = [];
    end

    methods
        function self = PunctureOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Ldpc.PunctureOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                self.mask = varargin{1};
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            else
                if (~isempty(varargin))
                    self.mask = varargin{1};
                end
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            end
        end
    end
end