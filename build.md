# Building for macos (M1 Chip)

This is a guide for building the project for macos (M1 Chip). It should be simliar to macos for intel chips but not sure. You may have to change some paths here and there.

## Installation

### Install vulkansdk

**Make sure you download version 1.3.211.0 or older.**

The newer versions will give vulkan driver error as the enuemration portability bit is not supported in the code base.

Create a folder at `$HOME/vulkansdk/1.3.211.0` (or the version you downloaded)

Install the vulkansdk for macos from [here](https://vulkan.lunarg.com/sdk/home).

Make sure to install everything except the global installation as we will be using env variables to point to the sdk.

### Set the environment variables for the sdk.

In your env rc file such as ~/.zshrc, paste the following:

```bash
export VULKAN_VERSION="1.3.211.0" #Version of sdk
export VULKAN_SDK="$HOME/development/vulkansdk/$VULKAN_VERSION/macOS" #Path to sdk
export PATH=$VULKAN_SDK/bin:$PATH
export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH
export VK_ICD_FILENAMES=$VULKAN_SDK/share/vulkan/icd.d/MoltenVK_icd.json
export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
```
**After adding the environment variables, make sure you restart the terminal.**

### Install homebrew

Install homebrew from [here](https://brew.sh/)

Install SDL2, SDL2_image, SDL2_ttf, SDL2_mixer dependencies and the gnu gcc compiler. You might have to install other dependencies/tools as well.

```bash
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf gcc
```

Add more environment variables for gcc

```bash
#m1 chip
export CPATH=":/opt/homebrew/include:$VULKAN_SDK/include"
export LIBRARY_PATH=":/opt/homebrew/lib:$VULKAN_SDK/lib"

#intel chip
export CPATH=":/usr/local/include:$VULKAN_SDK/include"
export LIBRARY_PATH=":/usr/local/lib:$VULKAN_SDK/lib"
```
**After adding the environment variables, make sure you restart the terminal.**

## Compiling gf3d

After cloning the repo, run the following command inside it:

`git submodule update --init --recursive`

Next go into each makefile and change the following:

- Compiler from `CC = gcc` to `CC = gcc-12` (or the version you installed)
to use gnu gcc compiler. 

- Change `-soname` to `-install_name` 

Then run `make` and `make static` inside each submodule.

Finally, run `make` inside `gf3d/src`

The executable should be in the `gf3d` folder.


### VK_NULL_HANDLE error
If you get the following error:
```
error: incompatible types when returning type 'void *' but 'VkFormat' was expected
   42 |             #define VK_NULL_HANDLE ((void*)0)
      |                                    ^
gf3d_pipeline.c:99:12: note: in expansion of macro 'VK_NULL_HANDLE'
   99 |     return VK_NULL_HANDLE;
      |            ^~~~~~~~~~~~~~
gf3d_pipeline.c:100:1: warning: control reaches end of non-void function [-Wreturn-type]
  100 | }
      | ^
make: *** [gf3d_pipeline.o] Error 1
```

Then you need to change the following in `gf3d/src/gf3d_pipeline.c` line 99 from `return VK_NULL_HANDLE;` to `return VK_FORMAT_UNDEFINED;`
