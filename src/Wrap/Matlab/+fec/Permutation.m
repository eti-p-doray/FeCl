%> This class represents a permutation vector.
%> A permutation generates a sequence of output data where each element
%> is picked at a specific index from the input sequence.
%> The index is defined by the index sequence given at the construction.
%> The permutation can permute many independant sequences at once.
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
        function output = dePermute(self, input)
            output = zeros(self.inputSize, size(input,2));
            output(self.sequence,:) = input;
        end
    end
end