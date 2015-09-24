
N = 8;
M = 2;
T = 32400;
snrdb = -5.0;
z = 1.96;

results.msgSize = T;
results.blocks = N;
results.experiments = M;

results.Convolutional = Convolutional(snrdb, T, N, M, z);
results.Turbo = Turbo(snrdb, T, N, M, z);
results.Ldpc = Ldpc(snrdb, T, N, M, z);

savejson('', results, '../benchmarkMatlabResult.json');

disp('done')

exit;