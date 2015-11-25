#/bin/sh -f

mkdir release
cd release
cp -rf ../Wrap/Matlab/+fec/ +fec
cp -rf ../example/Matlab/ example
zip -r FeCl-matlab.zip +fec example

cd ..
