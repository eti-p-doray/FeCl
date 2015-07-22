classdef TurboCode < fec.Code
    % This class is a specialization of the ErrorCorrectingCode.
    % It represents a Turbo code.

    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = fec.TurboCode();
            b.load(a);
        end
    end

    methods
        function this = TurboCode(trellis, interleaver, iterationCount, structureType, mapDecoderType, workGroupSize)
        % ConvolutionalCode constructore
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   trellis1 - Trellis structure used by the first code in the matlab communication system toolox form
        %   trellis2 - Trellis structure used by the second code in the matlab communication system toolox form
        %   interleaver - Interleaver used by the second code.
        %   [iterationCount] - Number of iterations. default = 5
        %   [trellisEndType] - trellis termination type. ZeroTail | Truncation default = Truncation
        %   [mapDecoderType] - decoder algorithm type. LogMap | MaxLogMap default = MaxLogMap
        %   [workGroupSize] - Number of thread used in decodage. default = 4
        %
          if (nargin == 0)
              return;
          end
          if (nargin < 4)
              iterationCount = 5;
          end
          if (nargin < 5)
            structureType = fec.StructureType.Serial;
          end
          if (nargin < 6)
            mapDecoderType = fec.MapType.MaxLogMap;
          end
          if (nargin < 7)
            workGroupSize = 4;
          end
          this.mexHandle_ = fec.bin.TurboCode_constructor(trellis, interleaver, iterationCount, structureType.char, mapDecoderType.char, workGroupSize);
        end
    end
end