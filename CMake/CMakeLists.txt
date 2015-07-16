cmake_minimum_required (VERSION 3.0)


add_library(ErrorCorrectingCode SHARED
	../ErrorCorrectingCode.cpp
	../CodeStructure/CodeStructure.cpp
	../CodeStructure/ConvolutionalCodeStructure.cpp
	../CodeStructure/TurboCodeStructure.cpp
	../CodeStructure/LdpcCodeStructure.cpp
	../CodeStructure/TrellisStructure.cpp
	../ConvolutionalCode/ConvolutionalCode.cpp
)