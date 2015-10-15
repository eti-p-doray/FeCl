import matlab.unittest.TestSuite
suiteClass = TestSuite.fromClass(?WrapTestClass);
resultTab = run(suiteClass);
resultTab = {resultTab.Failed}
result = 0;
for i = 1:length(resultTab)
    result = result || resultTab{i};
end
exit(result);