export MTL_HUD_ENABLED=1
export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH
make -C src/ && ./gf3d $1 $2
