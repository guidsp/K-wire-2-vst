# K-wire-2-vst
## Dependencies
VST3 SDK - https://github.com/steinbergmedia/vst3sdk
CMake - https://cmake.org/download/
## Build
- Edit the CMakeLists to point at the location of your VST3 SDK folder.
  - You may also set SMTG_CREATE_PLUGIN_LINK to 1 if you've already applied one of [these workarounds](https://steinbergmedia.github.io/vst3_dev_portal/pages/Getting+Started/Preparation+on+Windows.html).
- `mkdir build`
- `cd build`
- `cmake ..`
- Open the project and compile.
## About
K-wire 2 is a VST3 plug-in compressor with its ratio expressed as an attenuation multiplier ranging from 0x to 2x, meaning it can "over compress" and push the signal under the threshold.
