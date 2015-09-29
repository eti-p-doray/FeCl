classdef Permutation
    properties
        sequence;
        inputSize;
    end

    methods
        function self = Permutation(sequence, inputSize)
            if (nargin > 0)
                self.sequence = sequence;
                self.inputSize = inputSize;
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
            if (nargin == 3)
                output = varargin{1};
                output(varargin{2}.sequence,:) = varargin{3};
            end
        end
    end
end