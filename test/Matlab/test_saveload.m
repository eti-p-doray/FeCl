function success = test_saveload(codec)
    msgSize = codec.msgSize;
    a = codec.saveobj();
    codec = fec.Codec.loadobj(a);
    success = (msgSize == codec.msgSize);
end