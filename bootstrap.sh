#!/bin/bash

#
# Copyright SpiRITlab.
# https://github.com/SpiRITlab
#

# version of dependencies
CMAKE_Version=cmake-3.15.0
GMP_Version=gmp-6.2.0
NTL_Version=ntl-11.4.3
SEAL_Version=3.5.1
HElib_Version=v1.0.1

set_compilers() {
  GCCCompilers=$1
  if [ -z "$GCCCompilers" ] ; then
    echo "Please install GCC-9 or above."
    exit 1
  else
    LatestGCCVersionNum=$(printf "%d\n" "${GCCCompilers[@]}" | sort -rn | head -1)
    # Set compiler version
    CCcompiler=gcc-$LatestGCCVersionNum
    CPPcompiler=g++-$LatestGCCVersionNum
    echo "Set GCC and G++ compiler to $CCcompiler and $CPPcompiler."
  fi
}

OS=$(uname -s)
if [[ "$OS" == "Darwin" ]] ; then
  GCCCompilers=($(ls /usr/local/bin/g++-* | cut -d'-' -f2))
elif [[ "$OS" == "Linux" ]] ; then
  GCCCompilers=($(ls /usr/bin/g++-* | cut -d'-' -f2))
else
  echo 'Unsupported OS'
  exit 1
fi
set_compilers $GCCCompilers

# Directories and files
Marker=.install_ok              # marker to indicate whether a dependecy has installed properly
PROJECT_ROOT=`pwd`
DEPS_ROOT="$PROJECT_ROOT/deps"   # directory for installing dependencies
CMAKE_EXE=cmake

mkdir -p $DEPS_ROOT/{include,lib,bin,share,src,tmp}
DEPS_include=$DEPS_ROOT/include
DEPS_lib=$DEPS_ROOT/lib
DEPS_bin=$DEPS_ROOT/bin
DEPS_share=$DEPS_ROOT/share
DEPS_src=$DEPS_ROOT/src
DEPS_tmp=$DEPS_ROOT/tmp

set_trap() {
# exit when any command fails
set -e
# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' EXIT
}

parse_args() {
    IFS=',' read -ra OPT_PKGS <<< "$1"
    for i in "${OPT_PKGS[@]}"; do
        if [ "$i" == "help" ] ; then
            usage
            trap EXIT
            exit 0
        fi
    done
}

usage(){
    echo "$0 [AWSS3]"
}

set_trap
parse_args $1


# =============================================================================
# functions to minimize code redundancy
# =============================================================================
install_cmake() {
    echo "Installing $CMAKE..."
    wget https://github.com/Kitware/CMake/releases/download/v$(echo $CMAKE_Version | cut -d'-' -f2)/$CMAKE_Version.tar.gz
    tar -zxvf $CMAKE_Version.tar.gz
    rm $CMAKE_Version.tar.gz
    mv $CMAKE_Version $CMAKE
    cd $CMAKE
    ./bootstrap --prefix=$DEPS_ROOT
    make; make install
    echo "Installing $CMAKE... (DONE)"
    touch $Marker # add the marker
    cd ..
}

install_gmp() {
    echo "Installing $GMP..."
    wget https://ftp.gnu.org/gnu/gmp/$GMP_Version.tar.bz2
    tar jxf $GMP_Version.tar.bz2
    rm $GMP_Version.tar.bz2
    mv $GMP_Version $GMP
    cd $GMP
    ./configure --prefix=$DEPS_ROOT --exec-prefix=$DEPS_ROOT
    make; make install
    echo "Installing $GMP... (DONE)"
    touch $Marker # add the marker
    cd ..
}

install_ntl() {
    echo "Installing $NTL..."
    wget http://www.shoup.net/ntl/$NTL_Version.tar.gz
    tar xzf $NTL_Version.tar.gz
    rm $NTL_Version.tar.gz
    mv $NTL_Version $NTL
    cd $NTL/src
    ./configure TUNE=x86 DEF_PREFIX=$DEPS_ROOT PREFIX=$DEPS_ROOT NTL_GMP_LIP=on GMP_PREFIX=$DEPS_ROOT NTL_THREADS=on NTL_THREAD_BOOST=on NATIVE=off
    make CXXFLAGS="-fPIC -O3"
    make install
    echo "Installing $NTL... (DONE)"
    cd ..
    touch $Marker # add the marker
    cd ..
}

install_helib() {
    echo "Installing $HElib..."
    git clone https://github.com/SpiRITlab/HElib.git $HElib
    cd $HElib
    git checkout $HElib_Version
    $CMAKE_EXE -DCMAKE_INSTALL_PREFIX=$DEPS_ROOT .
    make LDLIBS+=-L$DEPS_lib CFLAGS+=-I$DEPS_include CFLAGS+=-fPIC
    cp -R include/helib $DEPS_include/
    cp lib/*.a $DEPS_lib/
    echo "Installing $HElib... (DONE)"
    touch $Marker # add the marker
    cd ..
}

install_base64() {
    echo "Installing $BASE64..."
    git clone https://github.com/SpiRITlab/libb64.git $BASE64
    cd $BASE64
    make
    cp -r include/b64/ $DEPS_include/b64
    cp src/libb64.a $DEPS_lib
    cp base64/base64 $DEPS_bin
    echo "Installing $BASE64... (DONE)"
    touch $Marker # add the marker
    cd ..
}

install_seal() {
    echo "Installing $SEAL..."
    git clone https://github.com/microsoft/SEAL.git $SEAL
    cd $SEAL
    git checkout $SEAL_Version
    $CMAKE_EXE -DCMAKE_INSTALL_PREFIX=$DEPS_ROOT .
    make LDLIBS+=-L$DEPS_lib CFLAGS+=-I$DEPS_include CFLAGS+=-fPIC
    make install
    echo "Installing $SEAL... (DONE)"
    touch $Marker # add the marker
    cd ..
}


# =============================================================================
# installing dependencies
# =============================================================================
cd $DEPS_src

export LD_LIBRARY_PATH=$DEPS_lib

# determine whether this script is ran inside a docker container
if [ ! -f "/proc/1/cgroup" ] || [ "$(grep 'docker\|lxc' /proc/1/cgroup)" == "" ] ; then
  # if not, install dependencies into a self-contained folder
  DEPS_ROOT=$DEPS_ROOT
else
  # if docker container, install dependencies into /usr/local
  DEPS_ROOT=/usr/local
fi


installMinRequiredCmake() {
  CMAKE="CMAKE"
  CMAKE_EXE=$DEPS_bin/cmake
  if [ -d $CMAKE ]; then
    if [ ! -f $CMAKE/$Marker ]; then
      rm -rf $CMAKE # remove the folder
      install_cmake
    else
      echo "$CMAKE library already installed"
    fi
  else
    install_cmake
  fi
}

echo "Checking CMake version...(Minimum required version: $(echo $CMAKE_Version | cut -d'-' -f2))"
if [ ! -z "$(cmake --version | grep 'version')" ]; then
  installedCmakeVersion=$(cmake --version | grep 'version' | cut -d' ' -f3)
  installedCmakeMajorVersion=$(echo $installedCmakeVersion | cut -d'.' -f1)
  installedCmakeMinorVersion=$(echo $installedCmakeVersion | cut -d'.' -f2)
  requiredMinCmakeMajorVersion=$(echo $CMAKE_Version | cut -d'-' -f2 | cut -d'.' -f1)
  requiredMinCmakeMinorVersion=$(echo $CMAKE_Version | cut -d'-' -f2 | cut -d'.' -f2)
  if [ "$installedCmakeMajorVersion" -lt "$requiredMinCmakeMajorVersion" ] \
    || [ "$installedCmakeMinorVersion" -lt "$requiredMinCmakeMinorVersion" ]; then
        echo "The installed version of CMake (current version: $installedCmakeVersion) is older than required. Perform install newer version."
        installMinRequiredCmake
  else
        echo "Minimum requirement for CMake is satisfied! (Current Version: $installedCmakeVersion)"
  fi
else
  installMinRequiredCmake
fi


# The GMP package contains math libraries. These have useful functions
# for arbitrary precision arithmetic.
GMP="GMP"
if [ -d $GMP ]; then
    if [ ! -f $GMP/$Marker ]; then
        rm -rf $GMP # remove the folder
        install_gmp
    else
        echo "$GMP already installed"
    fi
else
    install_gmp
fi


# NTL is a C++ library for doing number theory. NTL supports arbitrary
# length integer and arbitrary precision floating point arithmetic, finite
# fields, vectors, matrices, polynomials, lattice basis reduction and basic
# linear algebra.
NTL="NTL"
if [ -d $NTL ]; then
    if [ ! -f $NTL/$Marker ]; then
        rm -rf $NTL # remove the folder
        install_ntl
    else
        echo "$NTL already installed"
    fi
else
    install_ntl
fi


# HElib is a software library that implements homomorphic encryption (HE).
HElib="HElib"
if [ -d $HElib ]; then
     if [ ! -f $HElib/$Marker ]; then
        rm -rf $HElib # remove the folder
        install_helib
    else
        echo "$HElib already installed"
    fi
else
    install_helib
fi

# https://sourceforge.net/p/libb64/git
BASE64="BASE64"
if [ -d $BASE64 ]; then
    if [ ! -f $BASE64/$Marker ]; then
        rm -rf $BASE64 # remove the folder
        install_base64
    else
        echo "$BASE64 already installed"
    fi
else
   install_base64
fi

# https://www.microsoft.com/en-us/research/project/simple-encrypted-arithmetic-library/
# Recent version of SEAL will download and install gsl and zlib into its src folder.
SEAL="SEAL"
if [ -d $SEAL ]; then
    if [ ! -f $SEAL/$Marker ]; then
        rm -rf $SEAL # remove the folder
        install_seal
    else
        echo "$SEAL already installed"
    fi
else
   install_seal
fi



trap EXIT
