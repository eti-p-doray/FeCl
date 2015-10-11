classdef PunctureOptions < hgsetget
    properties
        mask;
        tailMask = [];
        bitOrdering = uint32(fec.Turbo.BitOrdering.('Alternate'));
    end

    methods
        function self = PunctureOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Turbo.PunctureOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            else
                if (~isempty(varargin))
                    self.mask = varargin{1};
                end
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            end
        end

        function self = set.bitOrdering(self,val)
            self.bitOrdering = uint32(fec.Turbo.BitOrdering.(val));
        end
    end
end