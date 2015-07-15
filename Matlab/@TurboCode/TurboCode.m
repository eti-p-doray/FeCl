classdef TurboCode < ErrorCorrectingCode
    % This class is a specialization of the ErrorCorrectingCode.
    % It represents a Turbo code.

    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = TurboCode();
            b.load(a);
        end
    end

    methods
        function this = TurboCode(trellis1, trellis2, interleaver, iterationCount, endType, mapDecoderType, workGroupSize)
        % ConvolutionalCode constructore
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   trellis1 - Trellis structure used by the first code in the matlab communication system toolox form
        %   trellis2 - Trellis structure used by the second code in the matlab communication system toolox form
        %   interleaver - Interleaver used by the second code.
        %   [iterationCount] - Number of iterations. default = 5
        %   [endType] - trellis termination type. ZeroTail | Truncation default = Truncation
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
            endType = BlocEndType.Truncation;
          end
          if (nargin < 6)
            mapDecoderType = MapType.MaxLogMap;
          end
          if (nargin < 7)
            workGroupSize = 4;
          end
          this.mexHandle_ = TurboCode_constructor(...
              trellis1.nextStates, trellis1.outputs, trellis1.numInputSymbols, trellis1.numOutputSymbols, trellis1.numStates,...
              trellis2.nextStates, trellis2.outputs, trellis2.numInputSymbols, trellis2.numOutputSymbols, trellis2.numStates,...
              interleaver, iterationCount, endType.char, mapDecoderType.char, workGroupSize);
        end
    end
end