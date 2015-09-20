#/bin/sh -f

# things to do for travis-ci in the before_install section

if ( test "`uname -s`" = "Darwin" )
then
  #cmake v2.8.12 is installed on the Mac workers now
  #brew update
  #brew install cmake
  echo
else
  #install a newer cmake since at this time Travis only has version 2.8.7
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
  echo "yes" | sudo add-apt-repository ppa:kalakris/cmake
  sudo apt-get update
  sudo apt-get install cmake
  sudo apt-get install g++-4.8
  export CXX="g++-4.8" CC="gcc-4.8"
  if [ "$CXX" = "clang++" ]; then sudo apt-get install -ibstdc++-4.8-dev; fi
  if [ "$CXX" = "g++" ]; then sudo apt-get install g++-4.8; fi
  if [ "$CXX" = "g++" ]; then export CXX="g++-4.8" CC="gcc-4.8"; fi
fi