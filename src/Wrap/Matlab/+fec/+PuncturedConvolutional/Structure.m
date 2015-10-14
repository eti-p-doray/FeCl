classdef Structure < fec.Convolutional.Structure
    properties
        punctureOptions = fec.Convolutional.PunctureOptions();
    end
    properties(Dependent = true)
        mask;
        tailMask;
    end

    methods
        function self = Structure(varargin)
            if (isa(varargin{1}, 'fec.Convolutional.EncoderOptions') || (isfield(varargin{1}, 'trellis') && isfield(varargin{1}, 'length')) || iscell(varargin{1}))
                self.encoderOptions = fec.Convolutional.EncoderOptions(varargin{1});
                if (nargin > 1)
                    self.punctureOptions = fec.Convolutional.PunctureOptions(varargin{2});
                end
                if (nargin > 2)
                    self.decoderOptions = fec.Convolutional.DecoderOptions(varargin{3});
                end
            else
                self.encoderOptions = fec.Convolutional.EncoderOptions(varargin{1}, varargin{2});
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

    end
end