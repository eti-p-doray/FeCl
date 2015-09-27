#/bin/sh -f

mkdir release
cd release
wget https://github.com/eti-p-doray/FeCl/archive/master.zip -O FeCl.zip
unzip FeCl.zip
cp -rf ../+fec/+bin/ FeCl-master/+fec/+bin/
zip -r ../FeCl.zip FeCl-master

cd ..
rm -r release/
