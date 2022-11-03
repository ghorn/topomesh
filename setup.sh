# system dependencies
sudo apt install -y build-essential zip libglm-dev gdal-bin jq lynx wget

# bazel
if which bazel ; then
    echo "bazel found"
else
    bazel_install_path="/usr/local/bin/bazel"
    bazelisk_url="https://github.com/bazelbuild/bazelisk/releases/download/v1.14.0/bazelisk-linux-amd64"

    echo "installing bazelisk as ${bazel_install_path}"
    sudo wget ${bazelisk_url} -O ${bazel_install_path}
    sudo chmod +x ${bazel_install_path}
fi
