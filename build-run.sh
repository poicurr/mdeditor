#!/bin/bash

ROOT_DIR=$(pwd)
BUILD_DIR=$ROOT_DIR/build/

function build {
  if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
  fi
  cd $BUILD_DIR
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1
  make
}

build

cd $ROOT_DIR
$BUILD_DIR/src/main.bin

