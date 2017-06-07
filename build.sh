#!/bin/bash

set -e

PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$PROJECT_ROOT/build"
CORE_COUNT="$( grep -c ^processor /proc/cpuinfo )"
THREAD_COUNT="$(($CORE_COUNT + 1))"

echo Project root: $PROJECT_ROOT, building in: $BUILD_DIR with $THREAD_COUNT threads

rm -rf "$BUILD_DIR" || true

mkdir "$BUILD_DIR"

pushd "$BUILD_DIR"
echo Building ServerAPP and ClientAPP
cmake ..
make -j $THREAD_COUNT

mkdir "$BUILD_DIR/GameAPP"
pushd GameAPP
echo Building GameAPP
qmake -qt=qt5 "$PROJECT_ROOT/GameAPP"
make -j $THREAD_COUNT
popd

mkdir "$BUILD_DIR/ChatAPP"
pushd ChatAPP
echo Building ChatAPP
qmake -qt=qt5 "$PROJECT_ROOT/ChatAPP"
make -j $THREAD_COUNT
popd

echo done
popd

cat <<EOF

====

You can start server with
$ cd build
$ ./ServerAPP/server 4200 keys -r replay

And chat client with
$ cd build
$ ./ClientAPP/client 127.0.0.1 4200

Note:
	ClientAPP/ServerAPP expects ChatAPP and GameAPP executables in
	'./chatAPP/chatAPP' and './GameAPP/Game' respectivly
	(relative to cwd)
EOF
