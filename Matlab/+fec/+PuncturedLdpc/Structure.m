classdef Structure < fec.Ldpc.Structure
    properties
        punctureOptions = fec.Ldpc.PunctureOptions();
    end
    properties(Dependent = true)
        mask;
        systMask;
    end

    methods
        function self = Structure(varargin)
            if (isa(varargin{1}, 'fec.Ldpc.EncoderOptions') || isfield(varargin{1}, 'checkMatrix') || iscell(varargin{1}))
                self.encoderOptions = fec.Ldpc.EncoderOptions(varargin{1});
                if (nargin > 1)
                    self.punctureOptions = fec.Ldpc.PunctureOptions(varargin{2});
                end
                if (nargin > 2)
                    self.decoderOptions = fec.Ldpc.DecoderOptions(varargin{3});
                end
            else
                self.encoderOptions = fec.Ldpc.EncoderOptions(varargin{1});
                if (~isempty(varargin(2:end)))
                    self.punctureOptions = fec.Ldpc.PunctureOptions(varargin{2});
                    if (~isempty(varargin(3:end)))
                        self.set(varargin{3:end});
                    end
                end
            end
        end
        
        function val = getPunctureOptions(self)
            val = self.punctureOptions.get();
        end

        function self = set.mask(self,val)
            self.punctureOptions.mask = val;
        end
        function self = set.systMask(self,val)
            self.punctureOptions.tailMask = val;
        end

    end
end