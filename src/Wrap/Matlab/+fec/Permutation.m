classdef Permutation
    properties
        sequence;
        inputSize;
    end

    methods
        function self = Permutation(sequence, inputSize)
            if (nargin > 0)
                self.sequence = sequence;
                if (nargin > 1)
                    self.inputSize = inputSize;
                else
                    self.inputSize = max(sequence);
                end
            end
        end

        function output = permute(self, input)
            output = input(self.sequence,:);
        end
        function output = dePermute(varargin)
            if (nargin == 2)
                output = zeros(varargin{1}.inputSize, size(varargin{2},2));
                output(varargin{1}.sequence,:) = varargin{2};
            end
        end
    end
end