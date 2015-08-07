ForwardErrorCorrection
=================

This software implements several forward error correction (fec) algorithms in C++ and offer a Matlab API.
Its main use is in communication systems simulations for the research area.

Implemented fec Codes:<br/>
Convolutional<br/>
Turbo (generalized to n codes)<br/>
Ldpc

A common interface is offered by every fec codes which makes the API more flexible and easier to use.

=================

Instructions:

To install the Matlab API <br/>
Download and run the toolbox file

You can build the source files for Matlab with mex. To do so, you need a Matlab compatible compiler. For release R2015a, see http://www.mathworks.com/support/compilers/R2015a/index.html <br/>
1-Download the project<br/>
2-Add the directory in your Matlab path<br/>
3-Run fec.make in the Matlab command prompt

You can build the source file as a static library with Cmake.
