cd $repos/ezira-node/dev ;
rm -r build/ ;
mkdir build && cd build ;
cmake \
	-DCMAKE_BUILD_TYPE=Debug \
	_DCFLAGS="-ferror-limit=0" \
	-DLOW_MEMORY_NODE=OFF \
	-DCLEAR_VOTES=OFF \
	-DSKIP_BY_TX_ID=ON \
	-DBUILD_EZIRA_TESTNET=OFF \
	-DEZIRA_STATIC_BUILD=ON \
	..

