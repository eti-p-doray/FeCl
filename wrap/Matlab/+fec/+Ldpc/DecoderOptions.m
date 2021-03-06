%>  This class gathers options affecting the decoder in Ldpc Codec.
classdef DecoderOptions < hgsetget
    properties
        %>  Maximum number of iterations in decoder.
        iterations = 5;
        %>  DecoderAlgorithm type used in decoder.
        algorithm = uint32(fec.DecoderAlgorithm.('Linear'));
        %>  Multiplicative scalingFactor in Approximate decoder.
%>  This can be a scalar, or a map in the form {key1, value1; key2 value2; ...}, where value, which will be used for the check degree defined by key, is a scalar or a vector with factors for every iterations. The key 0 is the default key which will be used for every degree that is not otherwise defined.
        scalingFactor = cell2struct({0, 1.0}, {'key', 'value'}, 2);
    end

    methods
        function self = DecoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Ldpc.DecoderOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
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
                self.algorithm = uint32(fec.DecoderAlgorithm.(val));
            else
                self.algorithm = uint32(fec.DecoderAlgorithm(val));
            end
        end
        function self = set.scalingFactor(self,val)
            if (isscalar(val))
                val = {0 val};
            end;
            if (iscell(val))
                self.scalingFactor = cell2struct(val, {'key', 'value'}, 2);
            end;
        end
    end
end