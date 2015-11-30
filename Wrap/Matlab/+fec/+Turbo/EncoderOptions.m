%>  This class gathers options affecting the encoder in Turbo Codec.
classdef EncoderOptions < hgsetget
    properties
        %>  Trellis structure used by each constituent.
        trellis;
        %>  Interleaver feeding every constituent.
        interleaver;
        %>  Convolutional::Termination type of each constituent.
        termination = {uint32(fec.Convolutional.Termination.('Tail'))};
    end

    methods
        function self = EncoderOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Turbo.EncoderOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            elseif (nargin == 1 && iscell(varargin{1}))
                varargin = varargin{1};
                self.trellis = varargin{1};
                self.interleaver = varargin{2};
                if (~isempty(varargin(3:end)))
                    self.set(varargin{3:end});
                end
            else
                self.trellis = varargin{1};
                self.interleaver = varargin{2};
                if (~isempty(varargin(3:end)))
                    self.set(varargin{3:end});
                end
            end
        end

        function self = set.trellis(self,val)
            if (length(val) == 1)
                self.trellis = {val};
            else
                self.trellis = val;
            end
        end
        function self = set.termination(self,val)
            if (~iscell(val))
                self.termination = {uint32(fec.Convolutional.Termination.(val))};
            else
                self.termination = cell(size(val));
                for i = 1:numel(val)
                    self.termination{i} = uint32(fec.Convolutional.Termination.(val{i}));
                end
            end
        end
    end
end