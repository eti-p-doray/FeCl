#/bin/sh -f

mkdir release
cd release
cp -rf ../wrap/Matlab/+fec/ +fec
cp -rf ../example/Matlab/ example
zip -r FeCl-matlab.zip +fec example

cd ..
