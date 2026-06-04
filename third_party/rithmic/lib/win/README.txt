Windows Rithmic Libraries
========================

Copy the following .lib files from the Rithmic API+ package (win10/lib/ or win7/lib/):

Required files (release + debug, 64-bit):
  RApiPlus_md64.lib     RApiPlus_mdd64.lib
  OmneStreamEngine_md64.lib  OmneStreamEngine_mdd64.lib
  OmneChannel_md64.lib      OmneChannel_mdd64.lib
  OmneEngine_md64.lib       OmneEngine_mdd64.lib
  api_md64.lib               api_mdd64.lib
  apistb_md64.lib            apistb_mdd64.lib
  kit_md64.lib               kit_mdd64.lib
  libssl_md64.lib            libssl_mdd64.lib
  libcrypto_md64.lib         libcrypto_mdd64.lib
  zlib_md64.lib              zlib_mdd64.lib

For 32-bit builds, also copy the non-64 variants (e.g. RApiPlus_md.lib).

Build commands:
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release

  cmake -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build --config Debug
