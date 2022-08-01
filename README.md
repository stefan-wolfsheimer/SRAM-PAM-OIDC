# Build
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
sudo apt install nlohmann-json-dev
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