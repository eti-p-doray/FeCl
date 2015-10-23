classdef Structure < fec.Turbo.Structure
    properties
        punctureOptions = fec.Turbo.PunctureOptions();
    end
    properties(Dependent = true)
        mask;
        tailMask;
        bitOrdering;
    end

    methods
        function self = Structure(varargin)
            if (isa(varargin{1}, 'fec.Turbo.EncoderOptions') || (isfield(varargin{1}, 'trellis') && isfield(varargin{1}, 'interleaver')))
                self.encoderOptions = fec.Turbo.EncoderOptions(varargin{1});
                if (nargin > 1)
                    self.punctureOptions = fec.Turbo.PunctureOptions(varargin{2});
                end
                if (nargin > 2)
                    self.decoderOptions = fec.Turbo.DecoderOptions(varargin{3});
                end
            else
                self.encoderOptions = fec.Turbo.EncoderOptions(varargin{1}, varargin{2});
                if (~isempty(varargin(3:end)))
                    self.set(varargin{3:end});
                end
            end
        end
        
        function val = getPunctureOptions(self)
            val = self.punctureOptions.get();
        end

        function self = set.mask(self,val)
            self.punctureOptions.mask = val;
        end
        function self = set.tailMask(self,val)
            self.punctureOptions.tailMask = val;
        end
        function self = set.bitOrdering(self,val)
            self.punctureOptions.bitOrdering = val;
        end

    end
end