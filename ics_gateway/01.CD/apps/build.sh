#!/bin/bash

help()
{
    echo "build.sh <target> <product_name> <major> <minor> <rev>"
    echo "     target             x86|raspi"
    echo "     product_name       the name of product"
    echo "Examples:"
    echo "     ./build.sh x86 DEMO 01 01 01"
    echo "     ./build.sh raspi ICS_GATEWAY 01 01 01"
}

ask()
{
    while [ 1 ]; do
        read -p "$1 (y/n)?" choice
        case "$choice" in 
            y|Y ) 
                return 0
                ;;
            n|N )
                return 1
                ;;
            * ) 
                echo "invalid";;
        esac
    done
}

if [ $# -ne 5 ]; then 
	help
	exit 1
fi

target=$1
product_name=$2
major=$3
minor=$4
rev=$5

export PRO_NAME=$product_name
export MAJOR=$major
export MINOR=$minor
export REV=$rev

set -e # exit when a command fails.
set -u # exit when using undeclared variables.

CUR_PATH=`pwd`

export OUTPUT_PATH=$CUR_PATH/output
echo "Target: $target"
echo "Product name: $product_name version: $major.$minor.$rev"
ask "Are you sure to continue compiling"
if [ $? -ne 0 ]; then
    exit 2
fi

echo "Clean up..."
rm -rf $OUTPUT_PATH/*
rm -rf iot-resource.tar.gz
mkdir -p $OUTPUT_PATH/bin
mkdir -p $OUTPUT_PATH/lib
mkdir -p $OUTPUT_PATH/util

rm -rf CMakeLists.txt
ln -s CMakeLists_${target}.txt CMakeLists.txt
echo "Generating make file..."
cmake .
make
make install

cp -rf $CUR_PATH/../3rd.party.${target}/lib/* $OUTPUT_PATH/lib

# echo "Generating SHA version"
# git rev-parse HEAD > $CUR_PATH/Util/SHA_Version.txt

echo "Collect util..."

cp -rf $CUR_PATH/Util/* $CUR_PATH/output/util
cp -rf $CUR_PATH/Config/* $CUR_PATH/output/util


echo "Collect 3rd bin..."
cp -rf $CUR_PATH/3rd/bin/* $CUR_PATH/output/bin

echo "Compress..."

cd $CUR_PATH/output
tar cvfz iot-resource-${target}.tar.gz  ./*

mv iot-resource-${target}.tar.gz $CUR_PATH
cd $CUR_PATH

echo "Done. Please check iot-resource-${target}.tar.gz"
exit 0