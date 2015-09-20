classdef DecoderOptions < matlab.mixin.SetGet
    properties
        iterations = 5;
        algorithm = uint32(fec.Codec.DecoderAlgorithm.('Linear'));
    end

    methods
        function self = DecoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Ldpc.DecoderOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                if (~isempty(varargin))
                    self.set(varargin{:});
                end
            else
                if (~isempty(varargin))
                    self.set(varargin{:});
                end
            end
        end

        function self = set.algorithm(self,val)
            if (ischar(val))
                self.algorithm = uint32(fec.Codec.DecoderAlgorithm.(val));
            else
                self.algorithm = uint32(fec.Codec.DecoderAlgorithm(val));
            end
        end
    end
end