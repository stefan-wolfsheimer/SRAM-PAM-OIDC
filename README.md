# Clone

Clone the repository and its submodules:
```
git clone https://github.com/stefan-wolfsheimer/SRAM-PAM-OIDC.git
git submodule update --init --recursive
```

# Build

## Build DEB package for ubuntu 20

Prepare build image
```
export PACKAGE_DIR=/path/to/target_package_dir
export BUILD_DIR=/path/to/build_dir

mkdir -p $PACKAGE_DIR
mkdir -p $BUILD_DIR

docker build -t sram-pam-oidc-builder \
             -f docker/Dockerfile.builder.ubuntu20 .
```

Compile and package
```
docker run --rm --workdir /build \
           -v $( pwd )/:/source \
           -v $BUILD_DIR/:/build \
           sram-pam-oidc-builder \
           cmake /source

docker run --rm --env VERBOSE=1 --workdir /build \
            -v $( pwd )/:/source \
            -v $BUILD_DIR/:/build \
            sram-pam-oidc-builder \
            make

docker run --rm --workdir /build \
       -v $( pwd )/:/source \
       -v $BUILD_DIR/:/build \
       -v $PACKAGE_DIR:/packages \
       sram-pam-oidc-builder cpack
```    

Launch interactive build system
```
docker run -ti --rm   \
       -v $( pwd ):/source \
       -v $PACKAGE_DIR:/packages \
       -v $BUILD_DIR/:/build sram-pam-oidc-builder bash
```

# Runner in a container

## Build
Build runner
```
docker build -t sram-pam-oidc-runner \
             -f docker/Dockerfile.runner.ubuntu20 .
```

## Configure
```
cp env.template env
```
and edit env

## Run PAM authentication flow
Interactive container
```
docker run -ti --rm --env-file env sram-pam-oidc-runner bash
```

single pam conversation
```
docker run --rm --env-file env sram-pam-oidc-runner sram_pamtester
```
