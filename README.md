![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

## Download

This repository uses [AppVeyor](https://www.appveyor.com) to automatically build every commit!
As such, you don't have to manually build any of the tools/libraries in this repository if you simply want to try out the latest versions.

- [Stable (release) builds](https://github.com/Radfordhound/HedgeLib/releases)
- [Latest (development) builds](https://ci.appveyor.com/project/Radfordhound/hedgelib)

## HedgeLib
HedgeLib is a C++ library [under the MIT license](License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens"](https://github.com/DarioSamo/libgens-sonicglvl).
Right now, HedgeLib supports the following:

### Hedgehog Engine
- "Mirage" headers (used in models/materials from Lost World and beyond)
- Models (.model/.terrain-model; v5.0)

### Sonic Forces
- Archives (.pac; aka "PACx V3")

### Sonic Lost World
- Archives (.pac; aka "PACx V2")

### Sonic Unleashed/Generations
- Uncompressed Archives (.ar/.pfd)

### Other
- BINA headers and string/offset tables (used in many files from '06, Colors, LW, Forces, and more)

## HedgeEdit
A level editor built with DirectX/Qt designed to carry much of the same functionality as "SonicGlvl", but with support for a large number of games in the series, rather than just Generations. Uses HedgeLib for reading/writing archives, models, materials, object placement, etc.

Right now it's a major WIP, though feel free to try it out!

## HedgeTools
A collection of miscellaneous tools that utilize HedgeLib to allow editing a wide array of formats.

Right now this includes the following:

- [HedgeArcPack](HedgeTools/HedgeArcPack)
  * A Command Line tool that allows quick and powerful extraction and generation of archiving formats from all games HedgeLib supports.
- [HedgeOffsets](HedgeTools/HedgeOffsets)
  * A Command Line tool that reads the offset table in BINA/Hedgehog Engine formats and prints each offset. Useful for debugging and reverse-engineering of undocumented formats which contain an offset table.

# Manually Building
Building HedgeLib is supposed to be easy.
If you're having trouble building with the following instructions, please [create an issue](https://github.com/Radfordhound/HedgeLib/issues/new).

1: Clone the HedgeLib repository:
```
git clone https://github.com/Radfordhound/HedgeLib.git
```

2: Download Premake from [here](https://premake.github.io/download.html) and extract it to the directory you cloned HedgeLib to.
HedgeLib uses Premake for easy Project/Makefile generation across different platforms/IDEs.

Everything that follows is platform-specific.

## Windows
You'll need to install the following if you don't have them already:

- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (Can also be downloaded from the Visual Studio Installer by selecting "Windows 10 SDK" from the "Individual Components" list)
- [Qt](https://www.qt.io/download)*

***NOTE:** Qt is only required for building HedgeEdit and the HedgeTools which have a GUI. It is NOT required to simply build HedgeLib or any of the command-line HedgeTools.


Once you've got everything, simply:

1: Open a cmd window in the folder you cloned HedgeLib to and run the following command:
```
premake5 vs2019
```

If you're using an older version of Visual Studio, simply replace ```vs2019``` with the version you're using.
(E.G. to use Visual Studio 2017, type ```premake5 vs2017``` instead.)

MinGW/Cygwin/etc. aren't officially supported/tested, but should also work fine. Just type ```premake5 gmake2``` instead.

2: Follow the on-screen prompts, if any.

3: Open the HedgeLib.sln file that Premake generated in Visual Studio and hit "Build Solution" (or Ctrl+Shift+B).

(Or, if you're using MinGW/Cygwin/etc., just run "make".)

## macOS
1: Open a terminal in the folder you cloned HedgeLib to and run the following command:
```
./premake5 xcode4
```

2: Follow the on-screen prompts, if any.

3: Open the resulting .xcworkspace in Xcode and build there.
Alternatively, you can build from the terminal by entering the following command:
```
xcodebuild
```

By default, this should compile a Debug build. To compile a Release build, simply run the following command instead:
```
xcodebuild -configuration Release
```

## Linux
1: Open a terminal in the folder you cloned HedgeLib to and run the following command:
```
./premake5 gmake2
```

If you wish to build using Clang instead of GCC, run ```./premake5 gmake2 --cc=clang``` instead.

2: Follow the on-screen prompts, if any.

3: Run the following command:
```
make config=debug_x64
```

To build with a different configuration, simply replace ```debug_x64``` with any of the following options:

- debug_x86     (Debug build for 32-bit x86 Architecture)
- debug_x64     (Debug build for 64-bit x86 Architecture)
- release_x86   (Release build for 32-bit x86 Architecture)
- release_x64   (Release build for 64-bit x86 Architecture)

Building for non-x86 architectures (such as ARM) is not officially supported.
If you'd like to help us change that, please let us know by [creating an issue](https://github.com/Radfordhound/HedgeLib/issues/new)!
