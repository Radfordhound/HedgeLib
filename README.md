![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

HedgeLib is a C/C++ library [under the MIT license](License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens".](https://github.com/DarioSamo/libgens-sonicglvl)

HedgeLib is fully C99-compatible, but also contains C++17-specific "extensions". This makes it possible to create bindings for other languages without sacrificing C++ usability. 

Right now, HedgeLib supports the following:

### Hedgehog Engine
- Reading Models.
- Reading "Mirage" headers.

### Sonic Lost World
- Reading Archives.

### Other
- Reading/Writing BINA headers/string tables/offset tables.

## HedgeEdit
A level editor built with DirectX/Qt designed to carry much of the same
functionality as "SonicGlvl", but with support for a large number of games in the series, rather than just Generations.

Right now it's a major WIP, though feel free to try it out!

## HedgeTools
A collection of miscellaneous tools that utilize HedgeLib to allow editing a wide array of formats.

Right now this includes the following:

- [Hedge Archive Packer](HedgeTools/HedgeArcPack)
  * A Command Line tool that allows quick and powerful editing of archiving formats from all games HedgeLib supports.
- [Hedge Offsets](HedgeTools/HedgeOffsets)
  * A Command Line tool that reads the offset table in BINA/Hedgehog Engine formats and prints each offset.

# Building
This repository uses [AppVeyor](https://www.appveyor.com/) to automatically build every commit!
As such, you don't have to manually build any of the tools/libraries in this repository if you simply want to try out the latest versions.

- [Stable (release) builds](https://github.com/Radfordhound/HedgeLib/releases) (none yet)
- [Latest (development) builds](https://ci.appveyor.com/project/Radfordhound/hedgelib)

## Manually building
If you do wish to manually build, however, simply follow the following instructions:

### Windows
Download and install the following (if you don't have them already):
- [Visual Studio 2017 (or later).](https://www.visualstudio.com/downloads/)
- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (for DirectX use in HedgeEdit; can be downloaded from Visual Studio).
- [Qt.](https://www.qt.io/download) (for HedgeEdit/HedgeTools with UIs; **not required for just building HedgeLib and tools without UIs**).

Then clone the repository and follow the following steps:

1: Download [Premake 5](https://premake.github.io/download.html#v5) and extract premake5.exe to the directory you cloned HedgeLib to.

2: Open a cmd window in the folder you cloned HedgeLib to and run the following command:
```
premake5 vs2017
```

If using Visual Studio 2019, simply replace ```vs2017``` with ```vs2019```.

3: If prompted, enter the location of your installed Qt 32-bit and 64-bit binaries for your target platform (e.g. C:\Qt\5.12.0\msvc2017 & C:\Qt\5.12.0\msvc2017_64).
A depends.lua file will be generated so you don't have to do this step again. Feel free to modify this file! You can also make a depends.lua manually and skip this step.

4: Open the HedgeLib.sln file that Premake generated in Visual Studio and hit "Build".

### Linux
Assuming you already have GCC 8 (or later), simply:

1: Download [Premake 5](https://premake.github.io/download.html#v5) and extract premake5 to the directory you cloned HedgeLib to.

2: Open a terminal in the folder you cloned HedgeLib to and run the following command:
```
./premake5 gmake2
```

3: Run the following command:
```
make config=debug_x64
```

To build with a different configuration, simply replace ```debug_x64``` with any of the following options:

- debug_x86     (Debug build for x86 Architecture)
- debug_x64     (Debug build for x64 Architecture)
- release_x86   (Release build for x86 Architecture)
- release_x64   (Release build for x64 Architecture)
