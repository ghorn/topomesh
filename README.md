This project downloads topographical data and converts it to meshes. I'm using https://github.com/fogleman/hmm to trianguate heightmaps found in various places.

Run the `setup.sh` script on Ubuntu to install bazel and system dependencies.

Read the `get_dem_data.sh` script FIRST and then run it to get DEMs.

Comment out the DEMs in BUILD.bazel that you don't want to build meshes for, then

> bazel build //...

![alt text](https://github.com/ghorn/topomesh/blob/main/readme/copernicus.png?raw=true)

![alt text](https://github.com/ghorn/topomesh/blob/main/readme/sfbay_reliefs.png?raw=true)
