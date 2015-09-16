classdef Codec < fec.MexObject
  % This abstract class represents a forward error correcting code.
  % It provide encoding and decoding method following a specific structure.
  % Several specialisations of this class define different structures
  % Methods of this class are implemented in cpp using mex to provide good
  % performances.
  % This class directly handles the allocated ressources in cpp.
    
    properties (Dependent = true)
        msgSize %Size of 1 msg bloc
        systSize %Size of 1 syst bloc
        paritySize %Size of 1 parity bloc
        stateSize %Size of 1 bloc of state information
        workGroupSize
    end
    
    methods (Static)
        function self = loadobj(s)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
            self.mexHanlde_ = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_load), s);
        end
    end
    
    methods
        function s = saveobj(self)
        % Overload of the saveobj
        %   Save configuration by serialization.
            s = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_save), self);
        end

        function val = get.msgSize(self)
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_msgSize), self);
        end
        function val = get.systSize(self)
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_systSize), self);
        end
        function val = get.stateSize(self)
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_stateSize), self);
        end
        function val = get.paritySize(self)
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_paritySize), self);
        end
        function val = get.workGroupSize(self)
            val = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_get_workGroupSize), self);
        end
        function set.workGroupSize(self, val)
            fec.bin.wrap(uint32(fec.WrapFcnId.Codec_set_workGroupSize), self, val);
        end

        function parity = encode(self, msg)
            parity = reshape(fec.bin.wrap(uint32(fec.WrapFcnId.Codec_encode), self, msg), [], size(msg,2));
        end

        function msg = decode(self, parity)
            msg = reshape(fec.bin.wrap(uint32(fec.WrapFcnId.Codec_decode), self, parity), [], size(parity,2));
        end
        function varargout = soDecode(self, parity, varargin)
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
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
            self.mexHandle_ = fec.bin.wrap(uint32(fec.WrapFcnId.Codec_load), s);
        end
    end
end