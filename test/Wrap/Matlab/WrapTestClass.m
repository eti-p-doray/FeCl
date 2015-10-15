classdef WrapTestClass < matlab.unittest.TestCase
    methods (Test)
        function testScalarConversion(testCase)
            res = testWrap(uint32(TestWrapFcnId.scalarConversion), 2);
            testCase.verifyEqual(res, 2);
        end
        function testVectorConversion(testCase)
            res = testWrap(uint32(TestWrapFcnId.vectorConversion), [0; 1; 2]);
            testCase.verifyEqual(res, [0; 1; 2]);
        end
        function testCellConversion(testCase)
            res = testWrap(uint32(TestWrapFcnId.vectorConversion), {0; 1; 2});
            testCase.verifyEqual(res, [0; 1; 2]);
        end
        function testTrellisConversion(testCase)
            trellis = fec.Trellis(4, 13, 15);
            trellis.numInputSymbols = double(trellis.numInputSymbols);
            trellis.numOutputSymbols = double(trellis.numOutputSymbols);
            trellis.numStates = double(trellis.numStates);
            trellis.nextStates = double(reshape(trellis.nextStates,[],2));
            trellis.outputs = double(reshape(trellis.outputs,[],2));
            testCase.verifyEqual(trellis, poly2trellis(4, 13, 15));
        end
    end
end