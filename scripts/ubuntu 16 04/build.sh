mkdir -p builds/ubuntu16
cd builds/ubuntu16
cmake -DCMAKE_BUILD_TYPE=Release ../..
make -j$(nproc)