#!/bin/bash




sudo apt-get -qq update
sudo apt-get install -y libassimp-dev libglm-dev graphviz libxcb-dri3-0 libxcb-present0 libpciaccess0 cmake libpng-dev libxcb-dri3-dev libx11-dev libx11-xcb-dev libmirclient-dev libwayland-dev libxrandr-dev
wget -O vulkansdk.tar.gz "https://sdk.lunarg.com/sdk/download/1.2.141.2/linux/vulkansdk-linux-x86_64-1.2.141.2.tar.gz?Human=true"
mkdir vulkansdk
tar -xzf vulkansdk.tar.gz --directory vulkansdk
ls vulkansdk/
export VK_SDK_PATH=$TRAVIS_BUILD_DIR/vulkansdk/1.2.141.2
export VULKAN_SDK="$VK_SDK_PATH/x86_64"
export PATH="$VULKAN_SDK/bin:$PATH"
export LD_LIBRARY_PATH="$VULKAN_SDK/lib:${LD_LIBRARY_PATH:-}"
export VK_LAYER_PATH="$VULKAN_SDK/etc/vulkan/explicit_layer.d"



echo $VULKAN_SDK
echo $VK_SDK_PATH
echo $PATH
python3 res.py
python3 gen.py
cd projects; ls -lah; make