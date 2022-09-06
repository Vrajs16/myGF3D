# Building for m1 chip

### All Makefiles

Make sure vulkan version for macos is 1.3.211.0 or less. Newer version will give vulkan driver error.

1. Change gcc to gcc-12
1. Change -soname to -install_name
1. Run `make clean` in each src
1. Run `make` in each src
1. Run `make static` only in all src folders in gfc submodule
