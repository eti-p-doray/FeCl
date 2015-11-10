% We are using the same trellis for every constituent
trellis = poly2trellis(4, 15, 17);

% We need 2 constituents per transmissions. To simulate 2 transmissions, we need 4 constituents.
% The two first constituent are only fed through interleavers with the first part 1:128 of the message sequence.
interl{1} = 1:128; % This interleaver is trivial
interl{2} = fec.Turbo.Lte3Gpp.interleaver(128); % We use an interleaver of length 128 defined in 3GPP LTE.
interl{3} = 1:192;
interl{4} = fec.Turbo.Lte3Gpp.interleaver(192);

% The codec is defined.
% Although it is possible to work with, tail bits are not included to simplify the example, as they make the rate harder to control. Therefore, the 'Truncate' value is given to the 'termination' parameter.
codec = fec.Turbo(trellis, interl, 'termination', 'Truncate');

% We defines indexing arrays to access the parity bits sent on the first and second transmissions through the puncturing of the mother codec.
parityIdx{1} = codec.puncturing('mask', {1:192<=128, 1, 1, 0, 0});
parityIdx{2} = codec.puncturing('mask', {1, 0, 0, [1 0], [1 0]});

% We defines indexing arrays to access the message bits contained in the first and second transmissions.
msgIdx{1} = fec.Permutation(1:128);
msgIdx{2} = fec.Permutation(1:192);

msg = randi([0 1], 192, 10);
parity = codec.encode(msg);

snrdb = [0.0, -1.0];
snr = 10.0 .^ (snrdb / 10.0);

llr = zeros(size(parity));
for i = 1:2
    symbol = parityIdx{i}.permute( (-2*double(parity)+1)*sqrt(2*snr(i)) );
    signal = symbol + randn(size(symbol)); % The received signal
    llr = llr + parityIdx{i}.dePermute(-4.0 * sqrt(snr(i)) * signal / sqrt(2.0));
    msgDec = codec.decode(llr);
    per(i) = sum( sum(msgIdx{i}.permute(msgDec ~= msg)) ~= 0 );
end