classdef (Abstract) WrapObject < handle
    properties (Hidden = true, SetAccess = protected)
        wrapHandle_ = int64(0); %handle to the underlying cpp ressources.
    end
end