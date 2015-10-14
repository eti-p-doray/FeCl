FeCl
=================

[![Build Status](https://travis-ci.org/eti-p-doray/FeCl.svg?branch=master)](https://travis-ci.org/eti-p-doray/FeCl)

FeCl is a channel coding library to help analysis of communication systems in research and education.
FeCl is licensed as free software under the lesser GPL license.
It can be used from C++ and Matlab.

Implemented fec Codecs<br/>
* Convolutional
* Turbo (generalized to n code constituents)
* Ldpc

A common interface is offered by every fec codes which makes the API more flexible and easier to use.
The implementation is optimized to process large amount of data. It profits from data alignement and parallelism.

=================

Documentation:

See example/ folder. You can use `doc` function in matlab after installation.
For C++ API, refer to the [web documentation](http://eti-p-doray.github.io/FeCl/)

=================

Instructions:

To install the Matlab API from pre-compiled binaries, download from [releases](https://github.com/eti-p-doray/FeCl/releases) and add the project to your Matlab path or move the project in your Matlab directory. <br/>
The toolbox is built for the following architectures<br/>
* win64 
* maci64 
* glnxa64 

You can build the source files as a library with cmake.
For an out-of-place build, run the following from the project directory<br/>
`mkdir build; cd build` <br/>
`cmake ..` <br/>
`make install` <br/>
