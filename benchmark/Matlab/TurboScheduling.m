function results = TurboScheduling(T, N, M)
    turboTrellis = poly2trellis(4, 15, 13);
    pi{1} = [];
    pi{2} = randperm(T);
    pi{3} = randperm(T);
    
    codec{1} = fec.Turbo(turboTrellis, pi, 'termination', 'Truncate', 'iterations', 10, 'scheduling', 'Serial', 'algorithm', 'Exact');
    codec{2} = fec.Turbo(turboTrellis, pi, 'termination', 'Truncate', 'iterations', 10, 'scheduling', 'Parallel', 'algorithm', 'Exact');
    
    config = {'Serial', 'Parallel'}; 
    for i = 1:2
        results.(config{i}) = simulation(codec{i}, codec{i}.puncturing('mask', [1 1 1; 1 0 0; 1 0 0; 1 0 0], 'bitOrdering', 'Group'), N, M, -3:0.1:-1.0);
    end
end