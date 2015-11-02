%>  This class gathers options affecting the decoder in Turbo Codec.
classdef DecoderOptions < hgsetget
    properties
        %>  Number of iterations in decoder.
        iterations = 5;
        %>  DecoderAlgorithm type used in decoder.
        algorithm = uint32(fec.DecoderAlgorithm.('Linear'));
        %>  Turbo::Scheduling type used in decoder.
        scheduling = uint32(fec.Turbo.Scheduling.('Serial'));
        %>  Multiplicative gain in Approximate decoder.
        gain = 1.0;
    end

    methods
        function self = DecoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Turbo.DecoderOptions'))
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
        function self = set.scheduling(self,val)
            if (ischar(val))
                self.scheduling = uint32(fec.Turbo.Scheduling.(val));
            else
                self.scheduling = uint32(fec.Turbo.Scheduling(val));
            end
        end
    end
end