classdef EncoderOptions < hgsetget
    properties
        trellis;
        length;
        termination = uint32(fec.Convolutional.Termination.('Tail'));
    end

    methods
        function self = EncoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Convolutional.EncoderOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                self.trellis = varargin{1};
                self.length = varargin{2};
                if (~isempty(varargin(3:end)))
                    self.set(varargin{3:end});
                end
            else
                self.trellis = varargin{1};
                self.length = varargin{2};
                if (~isempty(varargin(3:end)))
                    self.set(varargin{3:end});
                end
            end
        end

        function self = set.termination(self,val)
            self.termination = uint32(fec.Convolutional.Termination.(val));
        end
    end
end