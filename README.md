![HedgeLib Logo](Logo-small.png?raw=true "Thanks to Gotta Play Fast for the wonderful project logo!")
*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferrably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

HedgeLib is a C# library [under the MIT license](https://github.com/Radfordhound/HedgeLib/blob/master/License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens."](https://github.com/DarioSamo/libgens-sonicglvl)

Right now it supports the following:

## Havok
- Reading/Writing XML Havok data.

## Hedgehog Engine
- Reading/Writing lights.

## Sonic Unleashed/Sonic Generations
- Reading/Writing uncompressed archives.
- Reading set data.
- Reading/Writing terrain groups.
- Reading/Writing light lists.

## Sonic Colors/Sonic Lost World
- Reading/Writing BINA headers/footers.
- Reading/Writing GISM files.
- Reading/Writing set data.
- Reading/Writing SNDL (SouND List) files.

# HedgeEdit
**IMPORTANT:** Please compile HedgeLib using Visual Studio 2017 and the sln file provided before opening HedgeEdit in Unity!
If done correctly, Visual Studio should automatically copy the generated DLL file to the HedgeEdit Assets\Libraries folder.

Also make sure to open "debugTest" under "Assets\Scenes"!


A Unity-based level editor designed to carry much of the same functionality as "SonicGlvl," but with support for a large number of games in the series, rather than just Generations.

Right now it's a major WIP, though feel free to try it out!

# HedgeTools
A collection of miscellaneous tools that utilize HedgeLib to allow editing a wide array of formats.

Right now this includes the following:

- Hedge Archive Editor
  * A GUI/Command-Line tool that allows editing of archiving formats from all games HedgeLib supports.

- Hedge GISM Editor
  * A GUI tool that allows editing of the Lost World GISM format (basically a list of "Gismos" used by a stage).
