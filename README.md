ForwardErrorCorrection {#mainpage}
=================

This software implements several forward error correction (fec) algorithms in C++ and offer a Matlab API.
Its main use is in communication systems simulations for the research area.
This project is a result of my work at the emt research center at INRS, Quebec, Canada.

Implemented fec Codes<br/>
* Convolutional
* Turbo (generalized to n code constituents)
* Ldpc

A common interface is offered by every fec codes which makes the API more flexible and easier to use.
The implementation is optimized to process large amount of data. It profits from data alignement and parallelism.

=================

Instructions:

To install only the Matlab API from pre-compiled binaries, download and run the [toolbox file](https://github.com/eti-p-doray/ForwardErrorCorrection/blob/gh-pages/ForwardErrorCorrection.mltbx?raw=true) <br/>
Currently, the toolbox is built for the following architectures<br/>
* win64 
* maci64 
* glnxa64 

You can build the source files for Matlab with mex. To do so, you need a Matlab compatible compiler with c++11 support. For release R2015a, see <http://www.mathworks.com/support/compilers/R2015a/index.html> <br/>
1. Download the project<br/>
2. Add the directory in your Matlab path<br/>
3. Run `fec.make` in the Matlab command prompt<br/>

You can build the source files as a library with cmake.
For an out-of-place build, run the following from the project directory<br/>
`mkdir build; cd build` <br/>
`cmake ..` <br/>
`make install` <br/>

=================

Documentation:

Refer to the [web documentation](http://eti-p-doray.github.io/ForwardErrorCorrection/)