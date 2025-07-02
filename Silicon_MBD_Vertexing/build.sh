source /opt/sphenix/core/bin/sphenix_setup.sh -n new
cd module
mkdir install
mkdir build
cd build
../autogen.sh --prefix=$(pwd)/../install
make -j $(nproc) && make install
cd ../../
