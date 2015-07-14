classdef LdpcCode < ErrorCorrectingCode
    % This class represents a Turbo encoder / decoder pair

    methods

        function this = LdpcCode(H, iterationCount, decoderType, workGroupSize)
        % ConvolutionalCode constructore
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   trellis - Trellis structure used by the first encode / decoder in the matlab communication system toolox form
        %   blocSize - Size of each message bloc
        %   [endType] - trellis termination type. ZeroTail | Truncation default = Truncation
        %   [mapDecoderType] - decoder algorithm type. LogMap | MaxLogMap default = MaxLogMap
        %   [workGroupSize] - Number of thread used in decodage. default = 4
        %
          if (nargin < 2)
              iterationCount = 50;
          end
          if (nargin < 3)
            decoderType = BpType.MinSumBp;
          end
          if (nargin < 4)
            workGroupSize = 4;
          end
          this.mexHandle_ = LdpcCode_constructor(H, iterationCount, decoderType.char, workGroupSize);
        end
    end
end