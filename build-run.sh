#!/bin/bash

ROOT_DIR=$(pwd)
BUILD_DIR=$ROOT_DIR/build/

function clean {
  rm -rf $BUILD_DIR &>/dev/null
}

function build {
  if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
  fi
  cd $BUILD_DIR
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1
  make
}

# build C++ files
build

# copy bin
cp $ROOT_DIR/build/src/main.bin $ROOT_DIR/editor/bin/

# run
cd $ROOT_DIR/editor
node app.js

