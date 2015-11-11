
N = 128;
T = 32400;
z = 1.96;

results.msgSize = T;
results.blocks = N;

results.Convolutional = Convolutional(-2.8:0.1:-1.0, T, N, z);
results.Turbo = Turbo(-2.8:0.1:-1.0, T, N, z);
results.Ldpc = Ldpc(-2.8:0.1:-1.0, T, N, z);
%results.TurboScheduling = TurboScheduling(-2.2:0.05:-1.0, T, N, z);

savejson('', results, '../benchmarkMatlabResult.json');

disp('done')

exit;