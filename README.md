# Introducing Ezira (beta)

Ezira is a Delegated Proof of Stake blockchain that uses a "Proof of Brain" social consnensus algorithm for token allocation.

  - Currency symbol ECO.
  - Equity symbol ECO.
  - 10% APR inflation narrowing to 1% APR over 20 years.
  - 75% of inflation to "Proof of Brain" social consensus algorithm.
  - 15% of inflation to stake holders.
  - 10% of inflation to block producers.

# Public Announcement & Discussion

Ezira was announced on
[Ezira.io](https://ezira.io) prior to
the start of any mining.

# No Support & No Warranty

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

# Whitepaper

You can read the Ezira Whitepaper at [Ezira.io/ezira.pdf](https://ezira.io/whitepaper.pdf).

# Quickstart

Just want to get up and running quickly? We have pre-build docker images for your convenience. More details are in our [quickstart guide](https://github.com/eziranetwork/ezira/blob/master/doc/exchangequickstart.md).

# Building

We strongly recommend using one of our pre-built Docker images or using Docker to build Ezira. Both of these processes are described in the [quickstart guide](https://github.com/eziranetwork/ezira/blob/master/doc/exchangequickstart.md).

If you would still like to build from source, we do have [build instructions](https://github.com/eziranetwork/ezira/blob/master/doc/building.md) for Linux (Ubuntu LTS) and macOS X.

## Dockerized p2p Node

To run a p2p node (ca. 2GB of memory is required at the moment):

    docker run \
        -d -p 2001:2001 -p 8090:8090 --name node-default \
        eziranetwork/ezira

    docker logs -f node-default  # follow along

## Dockerized Full Node

To run a node with *all* the data (e.g. for supporting a content website)
that uses ca. 14GB of memory and growing:

    docker run \
        --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_CONTENT_NODE=1 \
        -d -p 2001:2001 -p 8090:8090 --name node \
        eziranetwork/ezira

    docker logs -f node

# Environment variables

There are quite a few environment variables that can be set to run node in different ways:

* `USE_WAY_TOO_MUCH_RAM` - if set to true, node starts a 'full node'
* `USE_FULL_CONTENT_NODE` - if set to true, a default config file will be used that enables a full set of API's and associated plugins.
* `USE_NGINX_FRONTEND` - if set to true, this will enable an NGINX reverse proxy in front of node that proxies websocket requests to node. This will also enable a custom healtcheck at the path '/health' that lists how many seconds away from current blockchain time your node is. It will return a '200' if it's less than 60 seconds away from synced.
* `USE_MULTICORE_READONLY` - if set to true, this will enable node in multiple reader mode to take advantage of multiple cores (if available). Read requests are handled by the read-only nodes, and write requests are forwarded back to the single 'writer' node automatically. NGINX load balances all requests to the reader nodes, 4 per available core. This setting is still considered experimental and may have trouble with some API calls until further development is completed.
* `HOME` - set this to the path where you want node to store it's data files (block log, shared memory, config file, etc). By default `/var/lib/node` is used and exists inside the docker container. If you want to use a different mountpoint (like a ramdisk, or a different drive) then you may want to set this variable to map the volume to your docker container.

# PaaS mode

Node now supports a PaaS mode (platform as a service) that currently works with Amazon's Elastic Beanstalk service. It can be launched using the following environment variables:

* `USE_PAAS` - if set to true, node will launch in a format that works with AWS EB. Containers will exit upon failure so that they can be relaunched automatically by ECS. This mode assumes `USE_WAY_TOO_MUCH_RAM` and `USE_FULL_CONTENT_NODE`, they do not need to be also set.
* `S3_BUCKET` - set this to the name of the S3 bucket where you will store shared memory files for node in Amazon S3. They will be stored compressed in bz2 format with the file name `blockchain-$VERSION-latest.tar.bz2`, where $VERSION is the release number followed by the git short commit hash stored in each docker image at `/etc/nodeversion`.
* `SYNC_TO_S3` - if set to true, the node will function to only generate shared memory files and upload them to the specified S3 bucket. This makes fast deployments and autoscaling for node possible.

# Seed Nodes

A list of some seed nodes to get you started can be found in
[doc/seednodes.txt](doc/seednodes.txt).

This same file is baked into the docker images and can be overridden by
setting `SEED_NODES` in the container environment at `docker run`
time to a whitespace delimited list of seed nodes (with port).

# Testing

See [doc/testing.md](doc/testing.md) for test build targets and info
on how to use lcov to check code test coverage.

# System Requirements

For a full web node, you need at least 270GB of disk space available. Node uses a memory mapped file which currently holds 189GB of data and by default is set to use up to 215GB. The block log of the blockchain itself is a little over 48GB. It's highly recommended to run node on a fast disk such as an SSD or by placing the shared memory files in a ramdisk and using the `--shared-file-dir=/path` command line option to specify where. At least 16GB of memory is required for a full web node. Seed nodes (p2p mode) can run with as little as 4GB of memory with a 24 GB state file. Any CPU with decent single core performance should be sufficient. Node is constantly growing. As of December 2017, these numbers were accurate, but you may find you need more disk space to run a full node. We are also constantly working on optimizing Ezira's use of disk space.

On Linux use the following Virtual Memory configuration for the initial sync and subsequent replays. It is not needed for normal operation.

```
echo    75 | sudo tee /proc/sys/vm/dirty_background_ratio
echo  1000 | sudo tee /proc/sys/vm/dirty_expire_centisec
echo    80 | sudo tee /proc/sys/vm/dirty_ratio
echo 30000 | sudo tee /proc/sys/vm/dirty_writeback_centisec
```

# Building Ezira

## Compile-Time Options (cmake)

### CMAKE_BUILD_TYPE=[Release/Debug]

Specifies whether to build with or without optimization and without or with
the symbol table for debugging. Unless you are specifically debugging or
running tests, it is recommended to build as release.

### LOW_MEMORY_NODE=[OFF/ON]

Builds node to be a consensus-only low memory node. Data and fields not
needed for consensus are not stored in the object database.  This option is
recommended for witnesses and seed-nodes.

### CLEAR_VOTES=[ON/OFF]

Clears old votes from memory that are no longer required for consensus.

### BUILD_TESTNET=[OFF/ON]

Builds Node for use in a private testnet. Also required for building unit tests.

### SKIP_BY_TX_ID=[OFF/ON]

By default this is off. Enabling will prevent the account history plugin querying transactions 
by id, but saving around 65% of CPU time when reindexing. Enabling this option is a
huge gain if you do not need this functionality.

## Building under Docker

We ship a Dockerfile.  This builds both common node type binaries.

    git clone https://github.com/eziranetwork/ezira
    cd ezira
    docker build -t eziranetwork/ezira .

## Building on Ubuntu 16.04

For Ubuntu 16.04 users, after installing the right packages with `apt` Ezira
will build out of the box without further effort:

    # Required packages
    sudo apt-get install -y \
        autoconf \
        automake \
        cmake \
        g++ \
        git \
        libssl-dev \
        libtool \
        make \
        pkg-config \
        python3 \
        python3-jinja2

    # Boost packages (also required)
    sudo apt-get install -y \
        libboost-chrono-dev \
        libboost-context-dev \
        libboost-coroutine-dev \
        libboost-date-time-dev \
        libboost-filesystem-dev \
        libboost-iostreams-dev \
        libboost-locale-dev \
        libboost-program-options-dev \
        libboost-serialization-dev \
        libboost-signals-dev \
        libboost-system-dev \
        libboost-test-dev \
        libboost-thread-dev

    # Optional packages (not required, but will make a nicer experience)
    sudo apt-get install -y \
        doxygen \
        libncurses5-dev \
        libreadline-dev \
        perl

    git clone https://github.com/eziranetwork/ezira
    cd ezira
    git submodule update --init --recursive
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc) node
    make -j$(nproc) cli_wallet
    # optional
    make install  # defaults to /usr/local

## Building on Ubuntu 14.04

(It is strongly advised to use Ubuntu 16.04 LTS instead)

Here are the required packages:

    # Required packages
    sudo apt-get install -y \
        autoconf \
        cmake3 \
        g++ \
        git \
        libssl-dev \
        libtool \
        make \
        pkg-config \
        doxygen \
        libncurses5-dev \
        libreadline-dev \
        libbz2-dev \
        python-dev \
        perl \
        python3 \
        python3-jinja2

The Boost provided in the Ubuntu 14.04 package manager (Boost 1.55) is too old.
Ezira requires Boost 1.58 (as in Ubuntu 16.04) and works with versions up to 1.60 (including).
So building Ezira on Ubuntu 14.04 requires downloading and installing a more recent
version of Boost.

According to [this mailing list
post](http://boost.2283326.n4.nabble.com/1-58-1-bugfix-release-necessary-td4674686.html),
Boost 1.58 is not compatible with gcc 4.8 (the default C++ compiler for
Ubuntu 14.04) when compiling in C++11 mode (which Ezira does).
So we will use Boost 1.60.

Here is how to build and install Boost 1.60 into your user's home directory
(make sure you install all the packages above first):

    export BOOST_ROOT=$HOME/opt/boost_1_60_0
    URL='http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.bz2/download'
    wget -c "$URL" -O boost_1_60_0.tar.bz2
    [ $( sha256sum boost_1_60_0.tar.bz2 | cut -d ' ' -f 1 ) == \
        "686affff989ac2488f79a97b9479efb9f2abae035b5ed4d8226de6857933fd3b" ] \
        || ( echo 'Corrupt download' ; exit 1 )
    tar xjf boost_1_60_0.tar.bz2
    cd boost_1_60_0
    ./bootstrap.sh "--prefix=$BOOST_ROOT"
    ./b2 install

Then the instructions are the same as for ezira:

    git clone https://github.com/eziranetwork/ezira
    cd ezira
    git submodule update --init --recursive
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc) node
    make -j$(nproc) cli_wallet

## Building on macOS X

Install Xcode and its command line tools by following the instructions here:
https://guide.macports.org/#installing.xcode.  In OS X 10.11 (El Capitan)
and newer, you will be prompted to install developer tools when running a
developer command in the terminal.

Accept the Xcode license if you have not already:

    sudo xcodebuild -license accept

Install Homebrew by following the instructions here: http://brew.sh/

### Initialize Homebrew:

   brew doctor
   brew update

### Install ezira dependencies:

    brew install \
        autoconf \
        automake \
        cmake \
        git \
        boost160 \
        libtool \
        openssl \
        python3 \
        python3-jinja2

Note: brew recently updated to boost 1.61.0, which is not yet supported by
ezira. Until then, this will allow you to install boost 1.60.0.

*Optional.* To use TCMalloc in LevelDB:

    brew install google-perftools

*Optional.* To use cli_wallet and override macOS's default readline installation:

    brew install --force readline
    brew link --force readline

### Clone the Repository

    git clone https://github.com/eziranetwork/ezira.git
    cd ezira

### Compile

    export OPENSSL_ROOT_DIR=$(brew --prefix)/Cellar/openssl/1.0.2h_1/
    export BOOST_ROOT=$(brew --prefix)/Cellar/boost@1.60/1.60.0/
    git submodule update --init --recursive
    mkdir build && cd build
    cmake -DBOOST_ROOT="$BOOST_ROOT" -DCMAKE_BUILD_TYPE=Release ..
    make -j$(sysctl -n hw.logicalcpu)

Also, some useful build targets for `make` are:

    node
    chain_test
    cli_wallet

e.g.:

    make -j$(sysctl -n hw.logicalcpu) node

This will only build `node`.

## Building on Other Platforms

- Windows build instructions do not yet exist.

- The developers normally compile with gcc and clang. These compilers should
  be well-supported.
- Community members occasionally attempt to compile the code with mingw,
  Intel and Microsoft compilers. These compilers may work, but the
  developers do not use them. Pull requests fixing warnings / errors from
  these compilers are accepted.
