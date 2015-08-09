classdef Code < handle
  % This abstract class represents a forward error correcting code.
  % It provide encoding and decoding method following a specific structure.
  % Several specialisations of this class define different structures
  % Methods of this class are implemented in cpp using mex to provide good
  % performances.
  % This class directly handles the allocated ressources in cpp.
  
    properties (Hidden = true, SetAccess = protected)
        mexHandle_ = int64(0); %handle to the underlying cpp ressources.
    end
    
    properties (Dependent = true)
        paritySize %Size of 1 parity bloc
        msgSize %Size of 1 msg bloc
        extrinsicSize %Size of 1 bloc of msg extrinsic information
    end
    
    methods (Static)
        function a = loadobj(b)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
            a = fec.Code();
            a.load(b);
        end
    end
    
    methods
        function size = get.msgSize(this)
        % Access size of the message in one bloc.
        %
        % Outputs
        %   size - Message size
            size = fec.bin.Code_get_msgSize(this);
        end
        function size = get.paritySize(this)
        % Access size of one parity bloc.
        %
        % Outputs
        %   size - Parity size
            size = fec.bin.Code_get_paritySize(this);
        end
        function size = get.extrinsicSize(this)
        % Access size of extrinsic information in one bloc.
        %
        % Outputs
        %   size - Extrinsic size
            size = fec.bin.Code_get_extrinsicSize(this);
        end
        
        function a = saveobj(this)
        % Overload of the saveobj
        %   Save configuration by serialization.
            a = fec.bin.Code_save(this);
        end
        function load(this, a)
        % Implementation of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            this.mexHandle_ = fec.bin.Code_load(a);
        end

        function delete(this)
        % ErrorCorrectingCode destructor
        %   Free underlying cpp ressources
            this.mexHandle_ = fec.bin.Code_destructor(this);
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
            parity = reshape(fec.bin.Code_encode(this, uint8(msg)), [], size(msg,2));
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
            msgOut = reshape(int8(fec.bin.Code_decode(this, double(parityIn))), [], size(parityIn,2));
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
            msgOut = reshape(fec.bin.Code_softOutDecode(this, double(parityIn)), [], size(parityIn,2));
        end
        
        function [msgOut, extrinsicOut] = appDecode(this, parityIn, extrinsicIn)
        % Decode several blocs of soft data according to the object code
        % structure,  using a priori (extrinsic) information about the whole decoder state (same parity)
        % and providing equivalent extrinsic L-values.
        % This method is usefull in ir Harq shemes where multiple transmission attempt can be done.
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
            [msgOut, extrinsicOut] = fec.bin.Code_appDecode(this, double(parityIn), double(extrinsicIn));
            msgOut = reshape(msgOut, [], size(parityIn,2));
            extrinsicOut = reshape(extrinsicOut, [], size(parityIn,2));
        end
    end
end