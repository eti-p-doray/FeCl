#/bin/sh -f

mkdir release
wget https://github.com/eti-p-doray/ForwardErrorCorrection/archive/master.zip -O release/fec.zip
unzip release/fec.zip -d release/
cp -rf +fec/+bin/ release/ForwardErrorCorrection-master/+fec/+bin/
zip -r fec.zip release/ForwardErrorCorrection-master

rm -r release/
