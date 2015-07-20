classdef ConvolutionalCode < fec.Code
    % This class is a specialization of the ErrorCorrectingCode.
    % It represents a Convolutional code.

    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = ConvolutionalCode();
            b.load(a);
        end
    end
  
    methods

        function this = ConvolutionalCode(trellis, blocSize, trellisEndType, mapDecoderType, workGroupSize)
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
          if (nargin == 0)
              return;
          end
          if (nargin < 3)
            trellisEndType = fec.TrellisEndType.Truncation;
          end
          if (nargin < 4)
            mapDecoderType = fec.MapType.MaxLogMap;
          end
          if (nargin < 5)
            workGroupSize = 4;
          end
          this.mexHandle_ = fec.bin.ConvolutionalCode_constructor(trellis.nextStates, trellis.outputs, trellis.numInputSymbols, trellis.numOutputSymbols, trellis.numStates,blocSize, trellisEndType.char, mapDecoderType.char, workGroupSize);
        end
    end
end