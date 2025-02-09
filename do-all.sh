#/bin/bash
BUILD_TYPE="Debug Release"
CONTAINER_TYPE="Host Alpine Ubuntu Debian Fedora"

for build in $BUILD_TYPE; do
  for container in $CONTAINER_TYPE; do
    ./configure.sh $build $container && make && make test && make package
  done
done