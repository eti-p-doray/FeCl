Installation
============

Depending on your needs, you may choose one of the following method.

## Matlab pre-compiled binaries
To install the Matlab API from pre-compiled binaries, download the most recent FeCl-matlab [release](https://github.com/eti-p-doray/FeCl/releases), which include the library (in a folder named +fec) and examples. For Matlab to recognize the library, you should either move the +fec folder anywhere you want and add it to your Matlab path (see [Change Folders on the Search Path](http://www.mathworks.com/help/matlab/matlab_env/add-remove-or-reorder-folders-on-the-search-path.html)), or move it to one of your Matlab directories (usually home/Matlab). <br/>
The toolbox is built for the following architectures<br/>
* win64 
* maci64 
* glnxa64 

## Matlab compilation
If the toolbox is not built for your architecture or you encounter problems with the binaries, you may try to build the project yourself. You first need a [Matlab compatible compiler](http://www.mathworks.com/support/compilers/R2015b/index.html). Download the most recent source code [release](https://github.com/eti-p-doray/FeCl/releases) and move the project anywhere you want. From Matlab, browse to the project and run the following in the command window <br/>
`run setup/install.m`<br/>

## Cpp compilation
You can build the source files for C++ (and Matlab) with [cmake](https://cmake.org/). Download the most recent source code [release](https://github.com/eti-p-doray/FeCl/releases). For an out-of-place build, run the following from the project directory<br/>
`mkdir build; cd build` <br/>
`cmake ..` <br/>
`make install` <br/>