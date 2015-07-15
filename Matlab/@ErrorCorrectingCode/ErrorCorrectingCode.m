classdef ErrorCorrectingCode < handle
  % This class represents a Convolutional encoder / decoder
  % Methods of this class are implemented in cpp using mex to provide good performances
  % This class directly handles the allocated ressources in cpp
  
    properties (Hidden = true, SetAccess = protected)
        mexHandle_ = nullHandle(); %handle to the underlying cpp ressources.
    end
    
    properties (Dependent = true)
        paritySize
        msgSize
        extrinsicMsgSize
        extrinsicParitySize
    end
    
    methods (Static)
        function a = loadobj(b)
            a = ErrorCorrectingCode();
            a.load(b);
        end
    end
    
    methods
        function size = get.msgSize(this)
            size = ErrorCorrectingCode_get_msgSize(this);
        end
        function size = get.paritySize(this)
            size = ErrorCorrectingCode_get_paritySize(this);
        end
        function size = get.extrinsicMsgSize(this)
            size = ErrorCorrectingCode_get_extrinsicMsgSize(this);
        end
        function size = get.extrinsicParitySize(this)
            size = ErrorCorrectingCode_get_extrinsicParitySize(this);
        end
        
        function a = saveobj(this)
        % Overload of the saveobj
        %   Free underlying ressources and save configuration.
        %   This prevents the underlying ressources from leaking when the object is saved
        %
            a = ErrorCorrectingCode_save(this);
        end
        function load(this, a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            this.mexHandle_ = ErrorCorrectingCode_load(a);
        end

        function delete(this)
        % MapCode destructor
        %   Free underlying ressources
            this.mexHandle_ = ErrorCorrectingCode_destructor(this);
        end
        
        function parity = encode(this, msg)
        % Encode several blocs of data according to the specified trellis structure
        %
        % Inputs
        % this - MapCode object
        % msg - Information bits to be coded
        %   This array is expected to have its number of rows equals to the blocSize specified in the constructor
        %
        % Outputs
        %   parity - Parity bits associated with the information bits
        %
            parity = reshape(ErrorCorrectingCode_encode(this, uint8(msg)), [], size(msg,2));
        end

        function msgOut = decode(this, parityIn)
        % Decode several blocs of soft data providing a posteriori information about the information and parity bits
        %
        % Inputs
        %   this - MapCode object
        %   parityExtrinsic - Extrinsic L-values associated with the parity bits
        %   msgExtrinsic - Extrinsic L-values associated with the message bits
        %
        % Outputs:
        %   msgAPosteriori - A posteriori L-values associated with the parity bits
        %   msgAPosteriori - A posteriori L-values associated with the message bits
        %
            msgOut = reshape(ErrorCorrectingCode_decode(this, double(parityIn)), [], size(parityIn,2));
        end
        
        function msgOut = softOutDecode(this, parityIn)
        % Decode several blocs of soft data providing a posteriori information about the information and parity bits
        %
        % Inputs
        %   this - MapCode object
        %   parityExtrinsic - Extrinsic L-values associated with the parity bits
        %   msgExtrinsic - Extrinsic L-values associated with the message bits
        %
        % Outputs:
        %   msgAPosteriori - A posteriori L-values associated with the parity bits
        %   msgAPosteriori - A posteriori L-values associated with the message bits
        %
            msgOut = reshape(ErrorCorrectingCode_softOutDecode(this, parityIn), [], size(parityIn,2));
        end
        
        function [msgOut, extrinsicOut] = appDecode(this, parityIn, extrinsicIn)
        % Decode several blocs of soft data providing a posteriori information about the information and parity bits
        %
        % Inputs
        %   this - MapCode object
        %   parityExtrinsic - Extrinsic L-values associated with the parity bits
        %   msgExtrinsic - Extrinsic L-values associated with the message bits
        %
        % Outputs:
        %   msgAPosteriori - A posteriori L-values associated with the parity bits
        %   msgAPosteriori - A posteriori L-values associated with the message bits
        %
            [msgOut, extrinsicOut] = ErrorCorrectingCode_appDecode(this, parityIn, extrinsicIn);
            msgOut = reshape(msgOut, [], size(parityIn,2));
            extrinsicOut = reshape(extrinsicOut, [], size(parityIn,2));
        end
        
        function [msgOut, extrinsicOut] = parityAppDecode(this, parityIn, extrinsicIn)
        % Decode several blocs of soft data providing a posteriori information about the information and parity bits
        %
        % Inputs
        %   this - MapCode object
        %   parityExtrinsic - Extrinsic L-values associated with the parity bits
        %   msgExtrinsic - Extrinsic L-values associated with the message bits
        %
        % Outputs:
        %   msgAPosteriori - A posteriori L-values associated with the parity bits
        %   msgAPosteriori - A posteriori L-values associated with the message bits
        %
            [msgOut, extrinsicOut] = ErrorCorrectingCode_parityAppDecode(this, parityIn, extrinsicIn);
            msgOut = reshape(msgOut, [], size(parityIn,2));
            extrinsicOut = reshape(extrinsicOut, [], size(parityIn,2));
        end
    end
end