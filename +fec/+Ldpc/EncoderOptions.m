classdef EncoderOptions < hgsetget
    properties
        checkMatrix;
    end

    methods
        function self = EncoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Ldpc.EncoderOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                self.checkMatrix = varargin{1};
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            else
                self.checkMatrix = varargin{1};
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            end
        end
    end
end