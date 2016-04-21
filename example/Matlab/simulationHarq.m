function simulationHarq
    trellis = poly2trellis(4, 15, 17);
    msgSize = 128;

    interl{1} = [];
    interl{2} = fec.Turbo.Lte3Gpp.interleaver(msgSize);
      
    codec = fec.Turbo(trellis, interl, 'termination', 'Truncate');

    snrdb1 = -0.0:1.0:10.0;
    snrdb2 = -0.0:1.0:10.0;
    snr = zeros(2, length(snrdb1), length(snrdb2));
    for i = 1:length(snrdb1)
        for j = 1:length(snrdb2)
            snr(1, i, j) = 10.0 ^ (snrdb1(i) / 10.0);
            snr(2, i, j) = 10.0 ^ (snrdb2(j) / 10.0);
        end
    end
    snr = reshape(snr, 2, []);

    parityPerm{1} = codec.puncturing('mask', [1 1; 1 0; 1 0]);
    parityPerm{2} = codec.puncturing('mask', [1 1; 0 1; 0 1]);

    msgPerm{1} = fec.Permutation(1:msgSize);
    msgPerm{2} = fec.Permutation(1:msgSize);
    
    result = harqPer(codec, msgPerm, parityPerm, snr, 64, 8);
    save('result', 'result', 'snr')
end



%%> This function computes the PER (probability of error) curve for multiple transmissions
%>   with differents snr. 16 QAM modulation is used.
%>
%>  @param  code  fec Code used for encoding and decoding
%>  @param  mIdx Cell array of array of index denoting bits of interest for the
%>       packet. This is used when multiple transmissions do not have the same R
%>       (such as in joint encoding)
%>  @param  cIdx  Cell array of array of index denoting the c bits sent
%>       a each transmission. This is used when multiple transmission do not
%>       send the same bits (such as in incrementl redundency)
%>  @param  snr This is a nxk array or function.
%>       For each n experiment, k transmissions will be executed with the corresponding snr.
%>       Example: for 2 transmission
%>
%>         | snr11 | snr12 | -> first transmission
%>         | snr21 | snr22 | -> second transmission
%>          1st exp 2nd exp
%>
%>  @param  N  Number of realisation in one bloc. A bloc is computed at once.
%>        Once the number of error reach N in an experiment (for a specific sequence of snr), this experiment stops.
%>  @param  M Number a bloc. Each bloc are computed sequentially.
%>  @param result Previous results obtained with the same setup. If
%>       available, new results will be combined.
%>
%>  @param[out] result Struture contaning results data. The field of this struture are defined as follow.
%>          experiments  Array of the number of realisation executed. This is a nxk array.
%>          per  Array of PER values. This is a nxk array.
%>          perCum  Array of cummulative PER values (pr{Nack}). This is a nxk array.
%>
function result = harqPer(codec, mIdx, cIdx, snr, N, M, result)
    if (nargin <= 6 || isempty(result))
        result.experiments = zeros(1, size(snr, 2));
        result.per = zeros(length(mIdx),size(snr, 2));
        result.perCum = zeros(length(mIdx),size(snr, 2));
    end
    transmissions = length(mIdx);

    hMod = comm.RectangularQAMModulator('ModulationOrder',16,'BitInput',true, 'NormalizationMethod', 'Average power');
    hDemod = comm.RectangularQAMDemodulator('ModulationOrder',16,'BitOutput',true, 'DecisionMethod', 'Approximate log-likelihood ratio', 'VarianceSource', 'Input port','NormalizationMethod', 'Average power');
    for i = 1:size(snr, 2)
        needToProcess = true;
        for j = 1:i-1
           if (all(snr(:,i) >= snr(:,j)) && all(result.per(end,j) == 0))
               needToProcess = false;
           end
        end
        if (~needToProcess)
            perCum(:,i) = 0;
            per(:,i) = 0;
            continue;
        end

        %i
        while ((result.experiments(i) < M*N) && (result.perCum(end,i) < N))
            
            % The m is randomly generated
            m = uint64(randi([0 1],codec.msgSize,N));
            % The code parities are generated
            c = double(codec.encode(m));
            % We execute a maximum number of M bloc of experiment.
            % If the number of error found exceeds N, the experiments stop
            % (we have enough error)
            result.experiments(i) = result.experiments(i)+N;
            errorCum = ones(1,N);
            llr = zeros(size(c));

            if (~iscell(cIdx))
                ci = cell(N,1);
                for k = 1:N
                    ci{k} = cIdx(k);
                end
            else
                for k = 1:N
                    ci{k} = cIdx;
                end
            end
            
            for k = 1:transmissions
            %We iterate over all tranmissions. k is the transmission number.
                %The cell array contains an array for each packet
                %containing Logicals denoting if each c bloc
                %contains an error at the k-th transmission.
                if (snr(k,i) < 0 || isnan(snr(k,i)))
                %If the snr is not a valid value, PER will be NaN.
                    perCum(k,i) = NaN;
                    per(k,i) = NaN;
                    continue;
                end
          
                
                for l = 1:N
                    parity = ci{l}{k}.permute(c(:,l));
                    x = hMod.step(parity);
                    if (snr(k,i) > 0)
                        y = x + complex(randn(size(x)),randn(size(x))) / sqrt(2*snr(k,i));
                        llr(:,l) = llr(:,l) - ci{l}{k}.dePermute(hDemod.step(y, 1/(2*snr(k,i))));
                    end
                end
                if (isempty(mIdx{k}.sequence))
                    continue;
                end
                
                %The m is decoded.
                mDec = codec.decode(llr);
               
                errorCum = errorCum & (sum(mIdx{k}.permute(mDec ~= m))~=0);
                result.perCum(k,i) = result.perCum(k,i) + sum(errorCum);
                result.per(k,i) = result.per(k,i) + sum(sum(mIdx{k}.permute(mDec ~= m))~=0);
                
            end
        end
    end
end