classdef ErrorCorrectingCode < handle
  % This abstract class represents a forward error correcting code.
  % It provide encoding and decoding method following a specific structure.
  % Several specialisations of this class define different structures
  % Methods of this class are implemented in cpp using mex to provide good
  % performances.
  % This class directly handles the allocated ressources in cpp.
  
    properties (Hidden = true, SetAccess = protected)
        mexHandle_ = nullHandle(); %handle to the underlying cpp ressources.
    end
    
    properties (Dependent = true)
        paritySize %Size of the 1 parity bloc
        msgSize %Size of the 1 msg bloc
        extrinsicMsgSize %Size of 1 bloc of msg extrinsic information
        extrinsicParitySize  %Size of 1 bloc of parity extrinsic information
    end
    
    methods (Static)
        function a = loadobj(b)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
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
        %   Save configuration by serialization.
            a = ErrorCorrectingCode_save(this);
        end
        function load(this, a)
        % Implementation of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            this.mexHandle_ = ErrorCorrectingCode_load(a);
        end

        function delete(this)
        % ErrorCorrectingCode destructor
        %   Free underlying cpp ressources
            this.mexHandle_ = ErrorCorrectingCode_destructor(this);
        end
        
        function parity = encode(this, msg)
        % Encode several blocs of data according to the object code
        % structure.
        %
        % Inputs
        %   this - Code object
        %   msg - Information bits to be coded
        %       This array is expected to have its number of rows equals to the
        %       msgSize.
        %       Each column corresponds to one bloc of data.
        %       Many msg blocs can be encoded at once.
        %
        % Outputs
        %   parity - Parity bits associated with the msg bits
        %
            parity = reshape(ErrorCorrectingCode_encode(this, uint8(msg)), [], size(msg,2));
        end

        function msgOut = decode(this, parityIn)
        % Decode several blocs of soft data according to the object code
        % structure.
        %
        % Inputs
        %   this - Code object
        %   parityIn - Parity L-values correspondig to the received signal.
        %       Given a signal y and a parity bit x, we define the correspondig L-value as
        %         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %       This array is expected to have its number of rows equals to the
        %       paritySize.
        %       Each column corresponds to one bloc of data.
        %       Many parity blocs can be decoded at once.
        %
        % Outputs:
        %   msgOut - Decoded msg
        %
            msgOut = reshape(ErrorCorrectingCode_decode(this, double(parityIn)), [], size(parityIn,2));
        end
        
        function msgOut = softOutDecode(this, parityIn)
        % Decode several blocs of soft data according to the object code
        % structure, providing a posteriori L-values.
        %
        % Inputs
        %   this - Code object
        %   parityIn - Parity L-values correspondig to the received signal.
        %       Given a signal y and a parity bit x, we define the correspondig L-value as
        %         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %       This array is expected to have its number of rows equals to the
        %       paritySize.
        %       Each column corresponds to one bloc of data.
        %       Many parity blocs can be decoded at once.
        %
        % Outputs:
        %   msgOut - Msg a posteriori L-values associated with the parity L-values.
        %
            msgOut = reshape(ErrorCorrectingCode_softOutDecode(this, parityIn), [], size(parityIn,2));
        end

        function [msgOut, extrinsicOut] = appDecode(this, parityIn, extrinsicIn)
        % Decode several blocs of soft data according to the object code
        % structure,  using a priori (extrinsic) information about the msg
        % and providing equivalent extrinsic L-values.
        %
        % Inputs
        %   this - Code object
        %   parityIn - Parity L-values correspondig to the received signal.
        %       Given a signal y and a parity bit x, we define the correspondig L-value as
        %         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %       This array is expected to have its number of rows equals to the
        %       paritySize.
        %       Each column corresponds to one bloc of data.
        %       Many parity blocs can be decoded at once.
        %   extrinsicIn - Extrinsic L-values associated with the msg.
        %
        % Outputs:
        %   msgOut - Msg a posteriori L-values associated with the parity L-values.
        %   extrinsicOut - Extrinsic L-values associated with the msg.
        %
            [msgOut, extrinsicOut] = ErrorCorrectingCode_appDecode(this, parityIn, extrinsicIn);
            msgOut = reshape(msgOut, [], size(parityIn,2));
            extrinsicOut = reshape(extrinsicOut, [], size(parityIn,2));
        end
        
        function [msgOut, extrinsicOut] = parityAppDecode(this, parityIn, extrinsicIn)
        % Decode several blocs of soft data according to the object code
        % structure,  using a priori (extrinsic) information about the whole decoder state (same parity)
        % and providing equivalent extrinsic L-values.
        % This method is usefull in ir Harq shemes where multiple transmissions can be executed.
        % Each transmission provides additional parities.
        % The decoder executes a decoding attempt on all the parity yet received,
        % but it can starts using the same state as where it left on the previous attempt.
        %
        % Inputs
        %   this - Code object
        %   parityIn - Parity L-values correspondig to the received signal.
        %       Given a signal y and a parity bit x, we define the correspondig L-value as
        %         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %       This array is expected to have its number of rows equals to the
        %       paritySize.
        %       Each column corresponds to one bloc of data.
        %       Many parity blocs can be decoded at once.
        %   extrinsicIn - Extrinsic L-values associated with the parities.
        %
        % Outputs:
        %   msgOut - Msg a posteriori L-values associated with the parity L-values.
        %   extrinsicOut - Extrinsic L-values associated with the parities.
        %
            [msgOut, extrinsicOut] = ErrorCorrectingCode_parityAppDecode(this, parityIn, extrinsicIn);
            msgOut = reshape(msgOut, [], size(parityIn,2));
            extrinsicOut = reshape(extrinsicOut, [], size(parityIn,2));
        end
    end
end