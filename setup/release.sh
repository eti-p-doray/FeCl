#/bin/sh -f

mkdir release
cd release
wget https://github.com/eti-p-doray/FeCl/archive/master.zip -O FeCl.zip
unzip -o FeCl.zip
rm FeCl.zip
cp -rf ../Matlab/+fec/+bin/ FeCl-master/Matlab/+fec/+bin/
cp -rf FeCl-master/Matlab/+fec/ +fec
zip -r FeCl-matlab.zip +fec

cd ..
