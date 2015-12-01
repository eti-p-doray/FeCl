%> @namespace fec
%> Foward Error Correction namespace.
%> This namespace groups everything contained in the library related to channel coding.


%> This class represents a general encoder / decoder.
%> It offers methods to encode and to decode data given a codec structure.
%> Several specialisations of this class define different codec types.
%> Methods of this class are implemented in cpp using mex to provide good
%> performances.
%> This class directly handles the allocated ressources in cpp.
classdef (Abstract) Codec < fec.detail.WrapObject
    
    properties (Dependent = true)
        %> Size of msg in each bloc
        msgSize
        %> Size of systematics in each  bloc
        systSize
        %> Size of parities in each  bloc
        paritySize
        %> Size of state information in each bloc
        stateSize
        %> Maximum of threads used in operations
        workGroupSize
    end
    properties (Hidden)
        structure
    end

    methods (Static)
        %> Overload of the loadobj method.
        %> Loads codec object from archive.
        function self = loadobj(s)
            self = fec.(s.class);
            self.reload(s);
        end
    end

    methods
        %> Overload of the saveobj method.
        %> Saves an archive of the object.
        function s = saveobj(self)
            s.archive = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_save), self);
            s.structure = self.structure;
            s.class = class(self);
            s.class = strrep(s.class, 'fec.', '');
        end

        %> Access the msgSize property
        function val = get.msgSize(self)
            val = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_get_msgSize), self);
        end
        %> Access the systSize property
        function val = get.systSize(self)
            val = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_get_systSize), self);
        end
        %> Access the stateSize property
        function val = get.stateSize(self)
            val = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_get_stateSize), self);
        end
        %> Access the paritySize property
        function val = get.paritySize(self)
            val = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_get_paritySize), self);
        end
        %> Access the workGroupSize property
        function val = get.workGroupSize(self)
            val = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_get_workGroupSize), self);
        end
        %> Modify the workGroupSize property
        function set.workGroupSize(self, val)
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_set_workGroupSize), self, val);
        end

        %>  Checks the consistency of a parity sequence.
        %>
        %>  @param parity Parity bits to be checked
        %>       This array is expected to have its number of rows equals to paritySize.
        %>       Each column corresponds to one bloc of data.
        %>       Many parity blocs can be checked at once.
        %>
        %>  @return True if parity sequences are consistent.
        function consistent = check(self, parity)
            consistent = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_check), self, parity);
        end

        %> Encode several blocs of data according to the object structure.
        %>
        %>  @param msg Information bits to be coded
        %>       This array is expected to have its number of rows equals to msgSize.
        %>       Each column corresponds to one bloc of data.
        %>       Many msg blocs can be encoded at once.
        %>
        %>  @return parity Parity bits encoding the msg bits
        %>
        %>  See example.
        %>  @snippet SimulationSimple.m  Encode
        function parity = encode(self, msg)
            parity = reshape(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_encode), self, msg), [], size(msg,2));
        end

        %> Decode several blocs of soft data according to the object structure.
        %>
        %>  @param parity - Parity L-values correspondig to the received signal.
        %>       Given a signal y and a parity bit x, we define the correspondig L-value as
        %>         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %>       This array is expected to have its number of rows equals to the
        %>       paritySize.
        %>       Each column corresponds to one bloc of data.
        %>       Many parity blocs can be decoded at once.
        %>
        %> @return Decoded msg
        %>
        %>  See example.
        %>  @snippet SimulationSimple.m  Decode
        function msg = decode(self, parity)
            msg = reshape(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_decode), self, parity), [], size(parity,2));
        end

        %> Decode several blocs of soft data according to the object structure, providing extrinsic information as outputs
        %>
        %>  @param parity A-priori L-values correspondig to the received signal.
        %>       Given a signal y and a parity bit x, we define the correspondig L-value as
        %>         L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
        %>       This array is expected to have its number of rows equals to the
        %>       paritySize.
        %>       Each column corresponds to one bloc of data.
        %>       Many parity blocs can be decoded at once.
        %>  @param [state] A-priori state information from previous decoding attempts.
        %>       Can be empty.
        %>  @param [syst] A-priori systematic bits information L-values.
        %>       Can be empty.
        %>
        %>  @param[out] msg A-posteriori msg information.
        %>  @param[out] [syst] Extrinsic systematic bits information.
        %>  @param[out] [state] Extrinsic state information.
        %>  @param[out] [parity] Extrinsic parity bits information.
        %>
        function varargout = soDecode(self, parity, varargin)
            varargout = cell(nargout,1);
            [varargout{:}] = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_soDecode), self, parity, varargin{:});
            for i = 1:nargout
                varargout{i} = reshape(varargout{i}, [], size(parity,2));
            end
        end

        %>  Free underlying cpp ressources
        function delete(self)
            self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_destroy), self);
        end
    end
    methods (Access = protected)
        function self = reload(self, s)
        % Implementation of the loadobj method
        %   Allocate underlying ressources from saved configuration.
            self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Codec_load), s.archive);
            self.structure = s.structure;
        end
    end
end