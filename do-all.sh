#/bin/bash
CONTAINER_TYPE="Host Alpine Ubuntu Debian Fedora"

build="Debug"
for container in $CONTAINER_TYPE; do
  if ./configure.sh $build $container && make && make test && make package; then
    echo "Build $build on $container succeeded"
  else
    echo "Build $build on $container failed"
    exit 1
  fi
done

build="Release"
for container in $CONTAINER_TYPE; do
  if ./configure.sh $build $container && make && make package; then
    echo "Build $build on $container succeeded"
  else
    echo "Build $build on $container failed"
    exit 1
  fi
done