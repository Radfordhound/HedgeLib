![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

HedgeLib is a C++17 library [under the MIT license](License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens".](https://github.com/DarioSamo/libgens-sonicglvl)

Right now it supports the following:

### Sonic Lost World
- Reading Archives.

### Other
- Reading/Writing BINA headers/footers.

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
If you do wish to manually build, however, simply download and install the following (if you don't have them already):
- [Visual Studio 2017 (or later).](https://www.visualstudio.com/downloads/)
- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (for DirectX use in HedgeEdit; can be downloaded from Visual Studio).
- [Qt.](https://www.qt.io/download)

Then clone the repository and follow the following steps:

1: Download [Premake 5](https://premake.github.io/download.html#v5) and extract premake5.exe to the directory you cloned HedgeLib to.

2: If you installed Qt anywhere other than the default directory ("C:/Qt/5.12.0") open HedgeEdit/depends.lua and change the two paths located there.
For example, if you installed Qt to "E:/Qt/5.12.0":

```lua
-- Change the following strings to whatever you want before generating
-- project files with premake to change where dependencies are located.
return {
	QtDir32 = "E:/Qt/5.12.0/msvc2017",
	QtDir64 = "E:/Qt/5.12.0/msvc2017_64"
}
```

3: Open a cmd window in the folder you cloned HedgeLib to and run the following command:
```
premake5 vs2017
```

4: Open the HedgeLib.sln file that Premake generated in Visual Studio and hit "Build".