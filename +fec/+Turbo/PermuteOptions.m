classdef PermuteOptions < hgsetget
    properties
        bitOrdering = uint32(fec.Turbo.BitOrdering.('Alternate'));
    end

    methods
        function self = PermuteOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Turbo.PermuteOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                if (~isempty(varargin))
                    self.set(varargin{1:end});
                end
            else
                if (~isempty(varargin))
                    self.set(varargin{:});
                end
            end
        end

        function self = set.bitOrdering(self,val)
            self.bitOrdering = uint32(fec.Turbo.BitOrdering.(val));
        end
    end
end