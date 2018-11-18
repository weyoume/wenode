git submodule update --init --recursive

mkdir -p builds/ubuntu
cd builds/ubuntu
cmake -DCMAKE_BUILD_TYPE=Release ../..
make -j$(nproc)