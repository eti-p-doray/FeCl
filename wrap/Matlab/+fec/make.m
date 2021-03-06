function make
% Build all required mex files for MapCode
% This function should be called once before any attempt to use MapCode
% The directoty path is automatically found
%   
    cxxFlags = ['-std=c++11  -fno-common -fexceptions -fPIC -pthread'];

    srcPath = '../../../src/';
    libsPath = '../../../libs/';
    trgPath = '../';

    iPath = {['-I' fullfile(srcPath)], ['-I' fullfile([libsPath 'include/'])]};
    lPath = {['-L' fullfile(libsPath)]};

    objDst = 'build';
    trgDst = '+bin';
    
    src = {...
        'Codec.cpp'; ...
        'Trellis.cpp'; ...
        'Convolutional.cpp'; ...
        'detail/Convolutional.cpp'; ...
        'detail/MapDecoder/MapDecoder.cpp'; ...
        'detail/MapDecoder/MapDecoderImpl.cpp'; ...
        'detail/ViterbiDecoder/ViterbiDecoder.cpp'; ...
        'detail/ViterbiDecoder/ViterbiDecoderImpl.cpp'; ...
        'Turbo.cpp'; ...
        'detail/Turbo.cpp'; ...
        'Lte3Gpp.cpp'; ...
        'detail/TurboDecoder/TurboDecoder.cpp'; ...
        'detail/TurboDecoder/TurboDecoderImpl.cpp'; ...
        'Ldpc.cpp'; ...
        'detail/Ldpc.cpp'; ...
        'DvbS2.cpp'; ...
        'detail/BpDecoder/BpDecoder.cpp'; ...
        'detail/BpDecoder/BpDecoderImpl.cpp'; ...
    };

    libs = {
        'serialization/src/archive_exception.cpp'; ...
        'serialization/src/basic_archive.cpp'; ...
        'serialization/src/basic_iarchive.cpp'; ...
        'serialization/src/basic_iserializer.cpp'; ...
        'serialization/src/basic_oarchive.cpp'; ...
        'serialization/src/basic_oserializer.cpp'; ...
        'serialization/src/basic_pointer_iserializer.cpp'; ...
        'serialization/src/basic_pointer_oserializer.cpp'; ...
        'serialization/src/basic_serializer_map.cpp'; ...
        'serialization/src/basic_text_iprimitive.cpp'; ...
        'serialization/src/basic_text_oprimitive.cpp'; ...
        'serialization/src/basic_xml_archive.cpp'; ...
        'serialization/src/binary_iarchive.cpp'; ...
        'serialization/src/binary_oarchive.cpp'; ...
        'serialization/src/codecvt_null.cpp'; ...
        'serialization/src/extended_type_info_no_rtti.cpp'; ...
        'serialization/src/extended_type_info_typeid.cpp'; ...
        'serialization/src/extended_type_info.cpp'; ...
        'serialization/src/polymorphic_iarchive.cpp'; ...
        'serialization/src/polymorphic_oarchive.cpp'; ...
        'serialization/src/stl_port.cpp'; ...
        'serialization/src/text_iarchive.cpp'; ...
        'serialization/src/text_oarchive.cpp'; ...
        'serialization/src/utf8_codecvt_facet.cpp'; ...
        'serialization/src/void_cast.cpp';...
        'serialization/src/xml_archive_exception.cpp'; ...
        'serialization/src/xml_iarchive.cpp'; ...
        'serialization/src/xml_oarchive.cpp'; ...
        'serialization/src/xml_grammar.cpp'; ...
        };
    
    trg = {...
        'wrap.cpp';...
        };
    
    
    
    oldpath = cd(strrep(which(fullfile('+fec', 'make.m')), fullfile('make.m'), ''));
    mkdir(objDst);
    mkdir(trgDst);
    
     objs = '';
    for i = 1:length(src)
        [pathstr,name,ext] = fileparts(src{i});
        objInfo = dir(fullfile(objDst, pathstr, [name '.*']));
        srcInfo = dir(fullfile(srcPath, src{i}));
        if (isempty(objInfo) || objInfo.datenum < srcInfo.datenum)
            mex('-v', ['CXXFLAGS= ' cxxFlags], iPath{:}, '-largeArrayDims', '-DBOOST_ALL_NO_LIB','-outdir', fullfile(objDst, pathstr), '-c', [srcPath src{i}]);
        else
            disp('skip');
        end
        objInfo = dir([fullfile(objDst, pathstr, [name '.*'])]);
        obj = fullfile(objDst, pathstr, objInfo.name);
        objs{length(objs)+1} = obj;
    end
    for i = 1:length(libs)
        [pathstr,name,ext] = fileparts(libs{i});
        objInfo = dir(fullfile(objDst, [name '.*']));
        srcInfo = dir(fullfile(libsPath, libs{i}));
        if (isempty(objInfo) || objInfo.datenum < srcInfo.datenum)
            mex('-v',['CXXFLAGS= ' cxxFlags], iPath{:}, '-largeArrayDims','-DBOOST_ALL_NO_LIB', '-outdir', objDst, '-c', [libsPath libs{i}]);
        else
            disp('skip');
        end
        objInfo = dir(fullfile(objDst, [name '.*']));
        obj = fullfile(objDst, objInfo.name);
        objs{length(objs)+1} = obj;
    end

    for i = 1:length(trg)
        trg{i} = [trgPath trg{i}];
    end

    mex('-v',['CXXFLAGS=' cxxFlags], iPath{:},lPath{:}, '-DBOOST_ALL_NO_LIB', '-largeArrayDims', '-outdir', trgDst, trg{:}, objs{:});
    
    cd(oldpath)
end
    