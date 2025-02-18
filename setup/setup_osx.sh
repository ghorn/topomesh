#!/usr/bin/env bash

# System dependencies. This is probably incomplete.
brew install gdal jq

# bazel
# get baselisk https://bazel.build/install/bazelisk
# probably from here: https://github.com/bazelbuild/bazelisk/releases
# and put it on your path
if which bazel ; then
    echo "bazel found"
else
    bazel_install_path="/usr/local/bin/bazel"
    bazelisk_url="https://github.com/bazelbuild/bazelisk/releases/download/v1.25.0/bazelisk-darwin"

    echo "installing bazelisk as ${bazel_install_path}"
    sudo wget ${bazelisk_url} -O ${bazel_install_path}
    sudo chmod +x ${bazel_install_path}
fi
