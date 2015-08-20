classdef LdpcCode < fec.Code
    % This class is a specialization of the Code abstract class.
    % It represents a Ldpc code.
    
    properties (Dependent = true)
        iterationCount
    end

    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = fec.LdpcCode();
            b.load(a);
        end
    end

    methods
        
        function count = get.iterationCount(this)
            count = fec.bin.LdpcCode_get_iterationCount(this);
        end
        function set.iterationCount(this, count)
            fec.bin.LdpcCode_set_iterationCount(this, count);
        end

        function this = LdpcCode(H, iterationCount, decoderType, workGroupSize)
        % LdpcCode constructor
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   H - Parity check matrix given in sparse of full form.
        %   [iterationCount] - Maximum number of iterations. If a consistent msg is found before the maximum number of
        %     iteration, the decoder will stop and the number of iteration is not reached. default = 50
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
            workGroupSize = 8;
          end
          this.mexHandle_ = fec.bin.LdpcCode_constructor(H, iterationCount, decoderType.char, workGroupSize);
        end
    end
end