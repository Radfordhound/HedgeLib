![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferrably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

HedgeLib is a C# library [under the MIT license](https://github.com/Radfordhound/HedgeLib/blob/master/License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens".](https://github.com/DarioSamo/libgens-sonicglvl)

Right now it supports the following:

### Havok
- [Reading/Writing XML Havok data.](HedgeLib/Collison/Havok.cs)

### Hedgehog Engine
- [Reading/Writing Lights.](HedgeLib/Lights/Light.cs)

### Sonic Heroes/Shadow the Hedgehog
- [Reading/Writing Uncompressed Archives.](HedgeLib/Archives/ONEArchive.cs)

### Sonic '06
- [Reading/Writing Set Data.](HedgeLib/Sets/S06SetData.cs)

### Sonic Unleashed/Sonic Generations
- [Reading/Writing Uncompressed Archives.](HedgeLib/Archives/GensArchive.cs)
- [Reading Set Data.](HedgeLib/Sets/GensSetData.cs)
- [Reading/Writing Terrain Groups.](HedgeLib/Terrain/GensTerrainGroup.cs)
- [Reading/Writing Light Lists.](HedgeLib/Lights/GensLightList.cs)

### Sonic Colors/Sonic Lost World
- [Reading/Writing BINA headers/footers.](HedgeLib/IO/BINA.cs)
- [Reading/Writing GISM files.](HedgeLib/Misc/LWGism.cs)
- [Reading/Writing Set Data.](HedgeLib/Sets/SOBJ.cs)
- [Reading/Writing SNDL (SouND List) files.](HedgeLib/Sound/ColorsSNDL.cs)

## HedgeEdit
A level editor built with OpenTK/Winforms designed to carry much of the same
functionality as "SonicGlvl", but with support for a large number of games in the series, rather than just Generations.

Right now it's a major WIP, though feel free to try it out!

## HedgeTools
A collection of miscellaneous tools that utilize HedgeLib to allow editing a wide array of formats.

Right now this includes the following:

- [Hedge Archive Editor](HedgeTools/HedgeArchiveEditor)
  * A GUI/Command-Line tool that allows editing of archiving formats from all games HedgeLib supports.

- [Hedge GISM Editor](HedgeTools/HedgeGISMEditor)
  * A GUI tool that allows editing of the Lost World GISM format (basically a list of "Gismos" used by a stage).

# Building
This repository uses [AppVeyor](https://www.appveyor.com/) to automatically build every commit!
As such, you don't have to manually build any of the tools/libraries in this repository if you simply want to try out the latest versions.

- [Stable (release) builds](https://github.com/Radfordhound/HedgeLib/releases)
- [Latest (development) builds](https://ci.appveyor.com/project/Radfordhound/hedgelib/build/artifacts)

## Manually building
If you do wish to manually build, however, all that's required is the following:
- [Visual Studio 2017 (or later).](https://www.visualstudio.com/downloads/)
- .NET Framework 4.6/4.7 (can be downloaded from Visual Studio).
- OpenTK and OpenTK.GLControl (both are automatically downloaded as a NuGet package when HedgeEdit is first opened in Visual Studio).
