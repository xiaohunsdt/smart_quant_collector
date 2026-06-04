macOS Rithmic static libraries
==============================

Copy the following .a files from your Rithmic API+ macOS package into this directory
(same layout as the vendor tree under lib/). Example source tree on this machine:

  /Volumes/MyNAS/source/rithmic/darwin-20.6-arm64/lib/

Use the directory that matches your OS version and CPU (e.g. darwin-20.6-arm64 for
Apple Silicon, or another darwin-* folder from the SDK).

Required files (must match names expected by CMake link line):

  libRApiPlus-optimize.a
  libOmneStreamEngine-optimize.a
  libOmneChannel-optimize.a
  libOmneEngine-optimize.a
  lib_api-optimize.a
  lib_apipoll-stubs-optimize.a
  lib_kit-optimize.a
  libssl.a
  libcrypto.a

zlib (-lz) is linked from the system SDK; it does not need to live here.

CMake uses only:

  ${CMAKE_SOURCE_DIR}/external/rithmic/lib/darwin

Do not point CMake at paths outside this project; keep all vendor binaries in this folder.

See also: ../win/README.txt for the Windows copy list.

Troubleshooting
---------------

CMake says the source directory does not match CMakeCache (e.g. project was configured
under /mnt/... but opened under /Volumes/...): delete the entire build directory and run
cmake again from the current path:

  rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

After upgrading conanfile.txt, CMake re-runs Conan automatically when the file changes.
If dependencies still look wrong, delete the build folder and reconfigure.

If the compiler reports missing standard headers ('string' file not found) on macOS,
ensure Command Line Tools or Xcode is installed and that CMake resolved the SDK (the
project fixes Conan setting CMAKE_OSX_SYSROOT to the shorthand "macosx" for Makefile builds).
