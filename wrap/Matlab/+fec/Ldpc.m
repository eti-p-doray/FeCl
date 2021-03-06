
%>  This class represents an ldpc encode / decoder.
%>  It offers methods encode and to decode data given an Structure.
%>
%>  The structure of the parity bits generated by an Ldpc object is as follow.
%>
%>    | syst | parity |
%>
%>  where syst are the systematic msg bits and parity are the added parity required
%>  to create a consistent bloc.
%>
%>  The structure of the extrinsic information is the case
%>
%>    | check1 | check2 | ... |
%>
%>  where checkX are the messages at the last iteration from the check node X
%>  that would be transmitted to the connected bit nodes at the next iteration.
classdef Ldpc < fec.Codec
    properties (Dependent = true, Hidden)
        encoderOptions
        decoderOptions
    end
    properties (Dependent = true)
        %>  Maximum number of iterations in decoder.
        iterations;
        %>  DecoderAlgorithm type used in decoder.
        algorithm;
    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Ldpc();
            self.reload(s);
        end
    end

    methods
        %>  Ldpc constructor.
        %>  `codec = fec.Ldpc(trellis, lenght, Name, Value)` creates an Ldpc object from a Trellis and lenght. Optionally, additional properties can be specified using Name (string inside single quotes) followed by its Value. See Ldpc::EncoderOptions and Ldpc::DecoderOptions for a list of properties.
        %>
        %> `codec = fec.Ldpc(encoderOptions, decoderOptions)` creates an Ldpc object from the Ldpc::EncoderOptions and the Ldpc::DecoderOptions structures, cell array or object containing encoder and decoder properties which describes the codec.
        %>
        %>  See example.
        %>  @snippet Ldpc.m  Creating a simple Ldpc Codec
        function self = Ldpc(varargin)
            if (nargin > 0)
              self.structure = fec.Ldpc.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_constructor), self.structure.getEncoderOptions, self.structure.getDecoderOptions);
            end
        end
        function perms = puncturing(self, varargin)
            options = fec.Ldpc.PunctureOptions(varargin{:});
            perms = fec.Permutation(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_puncturing), self, options.get()), self.paritySize);
        end
        function val = get.iterations(self)
            val = self.decoderOptions.iterations;
        end
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        function set.iterations(self, val)
            self.setDecoderOptions('iterations', val);
        end
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
        end
        function val = get.decoderOptions(self)
            val = self.structure.decoderOptions;
        end
        function val = get.encoderOptions(self)
            val = self.structure.encoderOptions;
        end
        function set.decoderOptions(self,val)
            self.structure.decoderOptions = fec.Ldpc.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_set_decoderOptions), self, decoderOptions.get());
        end
        %function set.encoderOptions(self,val)
        %    self.structure.encoderOptions = fec.Ldpc.EncoderOptions(val);
        %    fec.bin.wrap(uint32(fec.detail.WrapFcnId.Ldpc_set_encoderOptions), self, encoderOptions.get());
        %end
        function setDecoderOptions(self,varargin)
            decoderOptions = self.decoderOptions;
            decoderOptions.set(varargin{:});
            self.decoderOptions = decoderOptions;
        end
        %function setEncoderOptions(self,varargin)
        %    encoderOptions = self.encoderOptions;
        %    encoderOptions.set(varargin{:});
        %    self.encoderOptions = encoderOptions;
        %end
    end

end