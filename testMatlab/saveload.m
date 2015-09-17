function success = saveload(code)
    msgSize = code.msgSize;
    a = code.saveobj();
    code = fec.Code.loadobj(a);
    success = (msgSize == code.msgSize);
end