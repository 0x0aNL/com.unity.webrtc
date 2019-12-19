#!/bin/bash

export LIBWEBRTC_DOWNLOAD_URL=https://github.com/Unity-Technologies/com.unity.webrtc/releases/download/M72/webrtc-mac.zip
export SOLUTION_DIR=$(pwd)/Plugin~

# Install cmake
brew install cmake

# Download LibWebRTC 
curl -L $LIBWEBRTC_DOWNLOAD_URL > webrtc.zip
unzip -d $SOLUTION_DIR/webrtc webrtc.zip 

# Install googletest
git clone https://github.com/google/googletest.git
cd googletest
cmake .
make
sudo make install

# Build UnityRenderStreaming Plugin 
cd "$SOLUTION_DIR"
cmake -GXcode .
xcodebuild -scheme webrtc -configuration Release build
xcodebuild -scheme WebRTCPluginTest -configuration Release build

echo $BOKKEN_DEVICE_IP
echo $BOKKEN_DEVICE_IP
echo $BOKKEN_DEVICE_IP
echo $BOKKEN_DEVICE_IP

scp -i ~/.ssh/id_rsa_macmini -o "StrictHostKeyChecking=no" -r "$SOLUTION_DIR/WebRTCPluginTest/Release" bokken@$BOKKEN_DEVICE_IP:~/Test
ssh -i ~/.ssh/id_rsa_macmini -o "StrictHostKeyChecking=no" bokken@$BOKKEN_DEVICE_IP '~/Test/WebRTCPluginTest'

# Run UnitTest
#$SOLUTION_DIR/WebRTCPluginTest/Release/WebRTCPluginTest"