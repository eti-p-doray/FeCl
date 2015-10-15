function simulationHarq
    trellis = poly2trellis(4, 15, 17);
    constituentsLength = {128, 192};
    for i = 1:length(constituentsLength)
      interl{2*i-1} = 1:constituentsLength{i};
      interl{2*i} = fec.Turbo.Lte3Gpp.interleaver(constituentsLength{i});
    end
    codec = fec.Turbo(trellis, interl, 'termination', 'Truncate');

    snrdb = -5.0:1.0:2.0;
    snrdbTot = -5.0:1.0:2.0;

    snr = cell(2,1);
    for i = 1:length(snrdb)
        snr{1}(i) = 10.0 ^ (snrdb(i) / 10.0);
        for j = 1:length(snrdbTot)
            snr{2}(i,j) = 10.0 ^ (snrdbTot(j) / 10.0) - snr{1}(i);
        end
    end

    parityPerm{1} = codec.createPermutation('mask', {1:constituentsLength{2}<=constituentsLength{1}, [1 0], [1 0], 0, 0});
    parityPerm{2} = codec.createPermutation('mask', {1, 0, 0, [1 0], [1 0]});

    msgPerm{1} = fec.Permutation(1:128);
    msgPerm{2} = fec.Permutation(1:192);
    
    [per, perCum] = harqPer(codec, msgPerm, parityPerm, snr, 64, 8);
    save('data')
end



%> This function computes the PER (probability of error) curve for multiple transmissions
%>   with differents snr.
%>
%>  @param  code  fec Code used for encoding and decoding
%>  @param  msgPerm Cell array of array of index denoting bits of interest for the
%>       packet. This is used when multiple transmissions do not have the same R
%>       (such as in joint encoding)
%>  @param  parityPerm  Cell array of array of index denoting the parity bits sent
%>       a each transmission. This is used when multiple transmission do not
%>       send the same bits (such as in incrementl redundency)
%>  @param  snr Array of snr. The structure of snr is a kx1 cell array where k is the number of transmission.
%>       The i-th cell element is a i dimensional array with snr used for the i-th transmission.
%>       Example: for 2 transmission
%>
%>         | {1} |   {2}   | -> This is the transmission index
%>         | snr | snr snr | -> this represents two transmissions with the same snr1
%>         | snr | snr snr |
%>
%>  @param  N  Number of experiments in one bloc. A bloc is computed at once.
%>        Once the number of error reach N in an experiment (for a specific sequence of snr), this experiment stops.
%>  @param  M Number a bloc. Each bloc are computed sequentially.
%>
%>  @param[out]  per  Array of PER values. The structure is a kxk cell array where k is the number of transmission.
%>       The (i,j)-th cell element is a i-dimensional array with PER values for the i-th transmission
%>       about the j-th packet (only for joint codind, ignore j if not used)
%>       Refer to the snr structure.
%>  @param[out] perCum  Array of cummulative PER values (pr{Nack}).
%>       The structure is the same as per.
%>
function [per, perCum] = harqPer(codec, msgPerm, parityPerm, snr, N, M)

    % The msg is randomly generated
    msg = uint64(randi([0 1],codec.msgSize,N));
    % The code parities are generated
    parity = double(codec.encode(msg));
    
    % A recursive function is used to compute de PER curve
    [per, perCum] = harqPerImpl(length(msgPerm), codec, msgPerm, parityPerm, snr, msg, parity, M);
end

function [per, perCum] = harqPerImpl(transCount, codec, msgPerm, parityPerm, snr, msg, parity, M)
% This function computes the PER curve for multiple transmission
%   with different snr for a given code
%
% Inputs
%   tr - The transmission number at which the function is currently
%       exploring. This is the depth of the recursion.
%   transCount - The total number of transmission
%   code - fec Code used for encoding and decoding
%   msgPerm - Cell array of array of index denoting bits of interest for the
%       packet. This is used when multiple transmissions do not have the same R
%       (such as in joint encoding)
%   parityPerm - Cell array of array of index denoting the parity bits sent
%       a each transmission. This is used when multiple transmission do not
%       send the same bits (such as in incrementl redundency)
%   snr - Array of snr. See harqPer.
%   msg - Message used for experiment.
%   parity - Parity code associated with the msg.
%   M - Number a bloc. Each bloc are comouted sequentially.
%
% Outputs
%   per - Array of PER values. See harqPer.
%   perCum - Array of cummulative PER values.(pr{Nack}). See harqPer.
%

    %par for parallel. If we are using the parallel toolbox, array being accessed in parallel
    %must be cell arrays
    perPar = cell(size(snr{transCount}));
    snrPar = cell(size(snr{transCount}));
    perCumPar = cell(size(snr{transCount}));
    expCountPar = cell(numel(snr{transCount}));
    for i = 1:numel(snr{transCount})
        snrPar{i} = zeros(transCount,1);
        for k = 1:transCount
            snrPar{i}(k) = snr{k}(mod(i-1,numel(snr{k}))+1);
        end
    end
  
    parfor i = 1:numel(snr{transCount})
    %We iterate through all levels of snr in parallel
        disp(i);

        perPar{i} = zeros(transCount,transCount);
        perCumPar{i} = zeros(transCount,transCount);

        expCountPar{i} = 0;
        while ((expCountPar{i} < M) && (perCumPar{i}(transCount,transCount) < size(msg,2)))
            % We execute a maximum number of M bloc of experiment.
            % If the number of error found exceeds N, the experiments stop
            % (we have enough error)
            expCountPar{i} = expCountPar{i}+1;
            errorCum = cell(transCount,1);
            llr = zeros(size(parity));

            for k = 1:transCount
            %We iterate over all tranmissions. k is the transmission number.
                %The cell array contains an array for each packet
                %containing Logicals denoting if each parity bloc
                %contains an error at the k-th transmission.
                errorCum{k} = ones(1,size(msg,2));
                if (snrPar{i}(k) < 0 || isnan(snrPar{i}(k)))
                %If the snr is not a valid value, PER will be NaN.
                    for l = 1:k
                        perCumPar{i}(k,l) = NaN;
                        perPar{i}(k,l) = NaN;
                    end
                    continue;
                end
        
                %We are using bpsk modulation and only sending
                %parityPerm{k} at this transmission.
                symbol = (-2*parityPerm{k}.permute(double(parity))+1)*sqrt(2*snrPar{i}(k));
                signal = symbol + randn(size(symbol));
                % The L-values are addup with the previous one to
                % accumulate the information. This is equivalent to
                % maximum ration combining
                llr = llr + parityPerm{k}.dePermute(-4.0 * sqrt(snrPar{i}(k)) * signal / sqrt(2.0));

                %The msg is decoded.
                msgDec = codec.decode(llr);

                for l = 1:k
                %We iterate over all packets assigning the pER count
                    errorCum{l} = errorCum{l} & (sum(msgPerm{l}.permute(msgDec ~= msg))~=0);
                    for h = l+1:transCount
                        errorCum{h} = errorCum{l};
                    end
                    perCumPar{i}(k,l) = perCumPar{i}(k,l) + sum(errorCum{l});
                    perPar{i}(k,l) = perPar{i}(k,l) + sum(sum(msgPerm{l}.permute(msgDec ~= msg))~=0);
                end
            end
        end
    end

    per = cell(transCount,transCount);
    perCum = cell(transCount,transCount);
    expCount = cell(transCount,transCount);
    for j = 1:transCount
        for k = 1:j
            per{j,k} = zeros(size(snr{j}));
            perCum{j,k} = zeros(size(snr{j}));
            expCount{j,k} = zeros(size(snr{j}));
        end
    end
    for i = 1:numel(snr{transCount})
        for j = 1:transCount
            for k = 1:j
                per{j,k}(mod(i-1,numel(snr{j}))+1) = per{j,k}(mod(i-1,numel(snr{j}))+1) + perPar{i}(j,k);
                perCum{j,k}(mod(i-1,numel(snr{j}))+1) = perCum{j,k}(mod(i-1,numel(snr{j}))+1) + perCumPar{i}(j,k);
                expCount{j,k}(mod(i-1,numel(snr{j}))+1) = expCount{j,k}(mod(i-1,numel(snr{j}))+1) + expCountPar{i};
            end
        end
    end

    % The error count is divided by the total number of experiments.
    for j = 1:transCount
        for k = 1:j
            per{j,k} = per{j,k} ./ (expCount{j,k} * size(msg,2));
            perCum{j,k} = perCum{j,k} ./ (expCount{j,k} * size(msg,2));
        end
    end
end