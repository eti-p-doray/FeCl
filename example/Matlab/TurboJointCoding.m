 % We are using the same trellis for every constituent
 trellis = poly2trellis(4, 15, 17);

 % We need 2 constituents per transmission. To simulate 2 transmissions, we need 4 constituents.
 % The two first constituents are only fed through interleavers with the first part 1:128 of the message sequence.
 interl{1} = 1:128; % This interleaver is trivial
 interl{2} = fec.Turbo.Lte3Gpp.interleaver(128); % An interleaver defined in 3GPP LTE.
 
  % The two last constituents are fed with the entire message sequence, that is the concatenation of both packets.
 interl{3} = 1:192;
 interl{4} = fec.Turbo.Lte3Gpp.interleaver(192);

 % The codec is defined.
 % Although they are possible to work with, tail bits are not included to simplify the example, as they make the rate harder to control. Therefore, the 'Truncate' value is given to the 'termination' parameter.
 codec = fec.Turbo(trellis, interl, 'termination', 'Truncate');

 % We defines indexing arrays to access the parity bits sent on the first and second transmissions through the puncturing of the mother codec.
 parityIdx{1} = codec.puncturing('mask', {1:192<=128, 1, 1, 0, 0});
 parityIdx{2} = codec.puncturing('mask', {1, 0, 0, [1 0], [1 0]});

 % We defines indexing arrays to access the message bits contained in the first and second transmissions.
 msgIdx{1} = fec.Permutation(1:128);
 msgIdx{2} = fec.Permutation(1:192);
 
  % 10 blocs of data are randomly generated and encoded.
 u = randi([0 1], 192, 10);
 c = codec.encode(u);

 % A vector of snr that will be used for each transmission is created.
 snrdb = [0.0, -1.0];
 snr = 10.0 .^ (snrdb / 10.0);

 % The sequence of received L-values accumulates the new information at each transmission.
 llr = zeros(size(c));

 % In the case, we simulate two transmissions.
 for i = 1:2
     x = parityIdx{i}.permute(c);

     % The bpsk-modulated symbol sequence
     symbol = (-2*double(x)+1) * sqrt(2*snr(i));
     y = symbol + randn(size(symbol)); % The received signal

     % The received L-values is accumulated.
     llr = llr + parityIdx{i}.dePermute(-4.0 * sqrt(snr(i)) * y / sqrt(2.0));

     % The decoder attempts to decode from all L-values received so far.
     uDec = codec.decode(llr);

     % We compute the word error count.
     per(i) = sum( sum(msgIdx{i}.permute(uDec ~= u)) ~= 0 );
 end