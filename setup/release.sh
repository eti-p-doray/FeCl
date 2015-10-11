#/bin/sh -f

mkdir release
cd release
cp -rf ../Matlab/+fec/ +fec
zip -r FeCl-matlab.zip +fec

cd ..
