# Clone

Clone the repository and its submodules:
```
git clone https://github.com/stefan-wolfsheimer/SRAM-PAM-OIDC.git
git submodule update --init --recursive
```

# Build

## Build in container

### Build container

```
export PACKAGE_DIR=/path/to/target_package_dir
export BUILD_DIR=/path/to/build_dir

mkdir -p $PACKAGE_DIR
mkdir -p $BUILD_DIR

docker build -t sram-pam-oidc -f docker/Dockerfile.ubuntu20 .

docker run -ti --rm sram-pam-oidc  -v $( pwd )/src:/source -v $PACKAGE_DIR:/packages -v $BUILD_DIR/:/build bash

```


## Build requirements

* gcc / g++
* cmake
* libpam-dev
* libcurl4-openssl-dev

Optional:
* pamtester

Install requirements on Ubuntu

```
sudo apt install gcc
sudo apt install g++
sudo apt install cmake
sudo apt install libpam-dev
sudo apt install libcurl4-openssl-dev
sudo apt install python3-flask
```

Optional
```
sudo apt install pamtester
```

## Build
```
cmake  << PATH TO SRAM-PAM-OIDC >>
make
```

## Install

Ubuntu
```
sudo make install
```


Redhat
TODO