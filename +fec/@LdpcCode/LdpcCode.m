classdef LdpcCode < fec.Code
    % This class is a specialization of the ErrorCorrectingCode.
    % It represents a Ldpc code.

    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = LdpcCode();
            b.load(a);
        end
    end

    methods

        function this = LdpcCode(H, iterationCount, decoderType, workGroupSize)
        % LdpcCode constructor
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   H - Parity check matrix given in sparse of full form.
        %   [iterationCount] - Maximum number of iterations. default = 50
        %   [decoderType] - decoder algorithm type. MinSumBp | TrueBp default = MinSumBp
        %   [workGroupSize] - Number of thread used. default = 4
        %
          if (nargin == 0)
              return;
          end
          if (nargin < 2)
              iterationCount = 50;
          end
          if (nargin < 3)
            decoderType = fec.BpType.MinSumBp;
          end
          if (nargin < 4)
            workGroupSize = 4;
          end
          this.mexHandle_ = fec.bin.LdpcCode_constructor(H, iterationCount, decoderType.char, workGroupSize);
        end
    end
end