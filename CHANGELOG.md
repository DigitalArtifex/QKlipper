### 1.0.2

#### General
- Updated build system to CMake
- Updated Readme Installation and Usage instructions

#### Fixes
- Fixed bug in CMakeLists.txt when CMAKE_INSTALL_PREFIX was a list, causing the need to re-run configuration in a fresh build directory
- Fixed bug in CMakeLists.txt where the INSTALL_PREFIX was ending with '/'
- Fixed issue of missing resource for QKlipperCommand

#### New Features
- Added CMake MSVC detection and debugger properties
- Added support for Temperature, Controller and Heater fan types
- Added support for Generic Heaters
