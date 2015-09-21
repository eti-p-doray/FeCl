#/bin/sh -f

mkdir release
cd release
wget https://github.com/eti-p-doray/ForwardErrorCorrection/archive/master.zip -O fec.zip
unzip fec.zip
cp -rf ../+fec/+bin/ ForwardErrorCorrection-master/+fec/+bin/
zip -r ../fec.zip ForwardErrorCorrection-master

cd ..
rm -r release/
