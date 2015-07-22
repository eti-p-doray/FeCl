function succes = test_decode(code, msg)
    parity = code.encode(msg);
    decodedMsg = code.decode(parity);
    succes = (sum(decodedMsg - msg) == 0);
end