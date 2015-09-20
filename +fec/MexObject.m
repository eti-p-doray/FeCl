classdef MexObject < handle
    properties (Hidden = true, SetAccess = protected)
        mexHandle_ = int64(0); %handle to the underlying cpp ressources.
    end
end