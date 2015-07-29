function succes = test_saveload(code)
    msgSize = code.msgSize;
    a = code.saveobj();
    code = fec.Code.loadobj(a);
    succes = (msgSize == code.msgSize);
end