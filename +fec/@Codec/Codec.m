classdef Codec < fec.MexObject
  % This abstract class represents an fec codec.
  % It provides encoding and decoding method following a specific structure.
  % Several specialisations of this class define different codec types.
  % Methods of this class are implemented in cpp using mex to provide good
  % performances.
  % This class directly handles the allocated ressources in cpp.
    
    properties (Dependent = true)
        msgSize %Size of 1 msg bloc
        systSize %Size of 1 syst bloc
        paritySize %Size of 1 parity bloc
        stateSize %Size of 1 bloc of state information
        workGroupSize % Maximum of threads used in operations
    end
    
    methods
        function s = saveobj(self)
        % Overload of the saveobj method.
        %   Save configuration by serialization.
            s = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_save), self);
        end

        function val = get.msgSize(self)
        % Access the msgSize property
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_msgSize), self);
        end
        function val = get.systSize(self)
        % Access the systSize property
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_systSize), self);
        end
        function val = get.stateSize(self)
        % Access the stateSize property
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_stateSize), self);
        end
        function val = get.paritySize(self)
        % Access the paritySize property
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_paritySize), self);
        end
        function val = get.workGroupSize(self)
        % Access the workGroupSize property
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_workGroupSize), self);
        end
        function set.workGroupSize(self, val)
        % Modify the workGroupSize property
            fec.bin.wrap(uint32(fec.WrapFcnId.Codec_set_workGroupSize), self, val);
        end

        function parity = encode(self, msg)
        % Encode several blocs of data according to the object code
        % structure.
        %
        % Inputs
        %   self - Codec object
        %   msg - Information bits to be coded
        %       This array is expected to have its number of rows equals to
        %       msgSize.
        %       Each column corresponds to one bloc of data.
        %       Many msg blocs can be encoded at once.
        %
        % Outputs
        %   parity - Parity bits associated with the msg bits
        %
            parity = reshape(fec.bin.wrap(uint32(fec.WrapFcnId.Codec_encode), self, msg), [], size(msg,2));
        end

        function msg = decode(self, parity)
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
            msg = reshape(fec.bin.wrap(uint32(fec.WrapFcnId.Codec_decode), self, parity), [], size(parity,2));
        end
        function varargout = soDecode(self, parity, varargin)
        % Decode several blocs of soft data according to the object code
        % structure, providing extrinsic infomartion as outputs
        %
        % Inputs
        %   this - Code object
        %   parity - Parity extrinsic (a priori) L-values correspondig to the received signal.
        %       Given a signal y and a parity bit x, we define the correspondig L-value as
        %         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %       This array is expected to have its number of rows equals to the
        %       paritySize.
        %       Each column corresponds to one bloc of data.
        %       Many parity blocs can be decoded at once.
        %   [state] - State extrinsic (a priori) L-values from previous decoding attempts.
        %       Can be empty.
        %   [syst] - Systematic bits extrinsic (a priori) L-values.
        %       Can be empty.
        %
        % Outputs:
        %   [syst] - Systematic bits extrinsic (a posteriori) L-values.
        %   [state] - State extrinsic (a posteriori) L-values from previous decoding attempts.
        %   [parity] - Parity bits extrinsic (a posteriori) L-values.
        %
            varargout = cell(nargout,1);
            [varargout{:}] = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_soDecode), self, parity, varargin{:});
            for i = 1:nargout
                varargout{i} = reshape(varargout{i}, [], size(parity,2));
            end
        end
        
        function delete(self)
        %   Free underlying cpp ressources
            self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_destroy), self);
        end
        function self = reload(self, s)
        % Implementation of the loadobj method
        %   Allocate underlying ressources from saved configuration.
            self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_load), s);
        end
    end
end