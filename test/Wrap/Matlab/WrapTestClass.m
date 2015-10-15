classdef WrapTestClass < matlab.unittest.TestCase
    methods (Test)
        function testScalarConversion(testCase)
            val = 2;
            res = testWrap(uint32(TestWrapFcnId.scalarConversion), val);
            testCase.verifyEqual(res, val);
        end
        function testVectorConversion(testCase)
            val = [0; 1; 2];
            res = testWrap(uint32(TestWrapFcnId.vectorConversion), val);
            testCase.verifyEqual(res, val);
        end
    end
end