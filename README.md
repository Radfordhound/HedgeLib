![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

## Download

This repository uses [AppVeyor](https://www.appveyor.com) to automatically build every commit!
As such, you don't have to manually build any of the tools/libraries in this repository if you simply want to try out the latest versions.

- [Stable (release) builds](https://github.com/Radfordhound/HedgeLib/releases)
- [Latest (development) builds](https://ci.appveyor.com/project/Radfordhound/hedgelib)

## HedgeLib
HedgeLib is C++11 library [under the MIT license](License.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens"](https://github.com/DarioSamo/libgens-sonicglvl).
Right now, HedgeLib supports the following:

### Hedgehog Engine 1/2
- "Mirage" headers (used in models/materials from Lost World and beyond)
- Models (.model/.terrain-model; v5)
- Materials (.material; v1 and v3)
- Texsets (.texset; v0)
- Texture entries (.texture; v1)
- Terrain instance infos (.terrain-instanceinfo; v0 and v5)

**Used in:**
- Sonic Unleashed (360/PS3)
- Sonic Generations (PC/360/PS3)
- Sonic Lost World (PC/Wii U)
- Mario & Sonic at the Sochi 2014 Olympic Winter Games (Wii U)
- Mario & Sonic at the Rio 2016 Olympic Games (Wii U)
- Sonic Forces
- Olympic Games Tokyo 2020 - The Official Video Game
- Mario & Sonic at the Olympic Games Tokyo 2020
- Sakura Wars
- ...and more

### Ninja Next ("Sega NN")
- Node Motions (Skeletal Animations) [.xnm, .snm, .gnm, .cnm, .inm, .enm, .ynm, .znm]
- Camera Motions [.xnc, .snc, .gnc, .cnc, .inc, .enc, .ync, .znc]
- Light Motions [.xnl, .snl, .gnl, .cnl, .inl, .enl, .ynl, .znl]
- Morph Motions [.xnf, .snf, .gnf, .cnf, .inf, .enf, .ynf, .znf]
- Material Motions [.xnv, .snv, .gnv, .cnv, .inv, .env, .ynv, .znv]

**Used in:**
- Sega SuperStars
- Sonic Riders
- Sonic '06
- Super Monkey Ball: Banana Blitz
- Bleach: Shattered Blade
- Sonic and the Secret Rings
- Sonic Riders: Zero Gravity
- Sonic Unleashed (Wii/PS2)
- Sonic and the Black Knight
- Super Monkey Ball: Step and Roll
- Sonic the Hedgehog 4: Episode I
- Sonic Free Riders
- Sonic the Hedgehog 4: Episode II
- ...and more

### Other
#### PACx V403 Packfiles (.pac)
**Used in:**
- Puyo Puyo Tetris 2

#### PACx V402 Packfiles (.pac)
**Used in:**
- Olympic Games Tokyo 2020 - The Official Video Game
- Mario & Sonic at the Olympic Games Tokyo 2020
- Sakura Wars

#### PACx V3 Packfiles (.pac)
**Used in:**
- Sonic Forces

#### PACx V2 Packfiles (.pac)
**Used in:**
- Sonic Lost World (PC/Wii U)
- Mario & Sonic at the Rio 2016 Olympic Games (Wii U)

#### BINA
**Used in various files from:**
- Sonic '06
- Sonic Colors (Wii)
- Sonic Lost World (PC/Wii U)
- Mario & Sonic at the Rio 2016 Olympic Games (Wii U)
- Sonic Forces
- Mario & Sonic at the Olympic Games Tokyo 2020
- Sakura Wars
- ...and more

#### AR Archives (.ar/.pfd) & Packed File Indexes (.pfi)
**Used in:**
- Sonic Unleashed (360/PS3)
- Sonic Generations (PC/360/PS3)

## HedgeRender
A C++11 graphics/rendering engine designed around modern rendering APIs for speed and portability. Currently, it only supports Direct3D 11, but it has been designed from the ground up to work well with modern APIs such as Direct3D 12 and Vulkan, so porting it to those backends shouldn't be difficult.

## HedgeTools
A collection of miscellaneous tools that utilize HedgeLib to allow editing a wide array of formats.

Right now this includes the following:

- [HedgeEdit](HedgeTools/HedgeEdit)
  * A C++11 level editor built on top of HedgeLib and HedgeRender that I ultimately hope to allow for quick and powerful editing of levels from every mainline 3D Sonic game in the series (think "SonicGlvl" but for more than just Generations).
  Right now it's a major WIP, though feel free to try it out!
- [HedgeArcPack](HedgeTools/HedgeArcPack)
  * A C++17 Command-Line tool that allows quick and powerful extraction and generation of archiving formats from all games HedgeLib supports.
- [HedgeOffsets](HedgeTools/HedgeOffsets)
  * A C++11 Command-Line tool that reads the offset table in BINA/Hedgehog Engine formats and prints each offset. Useful for debugging and reverse-engineering of undocumented formats which contain an offset table.

# Manually Building
Building HedgeLib is supposed to be easy.
If you're having trouble building with the following instructions, please [create an issue](https://github.com/Radfordhound/HedgeLib/issues/new).

## Windows (Visual Studio)
**1: Install [CMake](https://cmake.org/download/) and [git](https://git-scm.com/download/win) if you haven't already.**

**IMPORTANT:** Be sure to select "Add CMake to the system PATH for all users" during the CMake installation, or you'll experience an error while following these steps.

**2: Open a Command Prompt or PowerShell window and clone the HedgeLib repository:**
```
git clone https://github.com/Radfordhound/HedgeLib.git
```

**3: Go into the HedgeLib folder and switch to the HedgeLib++ branch:**
```
cd HedgeLib
git switch HedgeLib++
```

**4: Generate a Visual Studio Solution (.sln file) with CMake:**
```
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ../
```

---

**NOTE:** If you got an error here like this:
```
'cmake' is not recognized as an internal or external command,
operable program or batch file.
```

It means you didn't choose the "Add CMake to the system PATH for all users" option during the CMake installation, so CMake is not on your system's PATH environment variable.

To fix this, just manually add the CMake bin folder (usually "C:\Program Files\CMake\bin") to your PATH environment variable (Google it if you don't know how). Please note that changes to the PATH variable won't take effect until you close and re-open your Command Prompt/PowerShell window, so make sure to do that (and then cd back into your HedgeLib/build directory, of course) before running CMake again.

Alternatively, you could also just use the already-installed CMake-gui program instead of using cmake from the command-line.

---

CMake will automatically download and setup any required dependencies as necessary and generate the required Visual Studio project and solution files.

**NOTE:** This command will generate solution and project files for Visual Studio 2019 that target 64-bit x86 processors.
If you wish to target a different Visual Studio version, replace ```Visual Studio 16 2019``` in the above command with any of the following values:

Value                     | Visual Studio Version
------------------------- | -----------------------------------------
Visual Studio 16 2019     | Visual Studio 2019
Visual Studio 15 2017     | Visual Studio 2017
Visual Studio 14 2015     | Visual Studio 2015
Visual Studio 12 2013     | Visual Studio 2013
Visual Studio 11 2012     | Visual Studio 2012
Visual Studio 10 2010     | Visual Studio 2010
Visual Studio 9 2008      | Visual Studio 2008

If you wish to target a different architecture, replace ```x64``` in the above command with any of the following values:

Value   | Architecture
------- | -------------------------
Win32   | 32-bit x86 Architecture
x64     | 64-bit x86 Architecture
ARM     | 32-bit ARM Architecture
ARM64   | 64-bit ARM Architecture

**5: Open the resulting HedgeLib.sln file (contained within the "build" directory) in Visual Studio and click "Build" -> "Build Solution"** (or press Ctrl+Shift+B).

If building succeeds, the resulting .exe files will be located within the build/**(program name)**/**(configuration)** directory (e.g. build/HedgeArcPack/Debug/HedgeArcPack.exe)

## macOS
**1: Open a terminal and install Homebrew (if you don't already have it):**
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**2: Install the required dependencies:**
```
brew install cmake git glm lz4 zlib
```

**3: Clone the HedgeLib repository:**
```
git clone https://github.com/Radfordhound/HedgeLib.git
```

**3: Go into the HedgeLib folder and switch to the HedgeLib++ branch:**
```
cd HedgeLib
git switch HedgeLib++
```

**4: Generate Makefiles or Xcode project files with CMake:**


To generate Makefiles:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../
```

---

**NOTE:** The generated Makefile(s) will produce a Debug build.
To change this, replace ```Debug``` in the above command with any of the following values:


Value           | Meaning
--------------- | -----------------------------------------------------------------------------
Debug           | Debug build; has debugging information (such as symbols) and no optimization.
Release         | Release build; is fully optimized and has no debugging information.
RelWithDebInfo  | Release build with debugging information.
MinSizeRel      | Release build, but optimized for size rather than speed.

---

To generate Xcode project files:
```
mkdir build
cd build
cmake -G "Xcode" ../
```

---

**NOTE:** If you get this weird error:
```
CMake Error: Xcode 1.5 not supported.
```

It means CMake isn't finding the correct Xcode installation. To fix it, run the following command, but with "/Applications/Xcode_10.1.app" replaced with the path to your machine's Xcode.app file:
```
sudo /usr/bin/xcode-select --switch /Applications/Xcode_10.1.app
```

Then, run CMake again:
```
cmake -G "Xcode" ../
```

---

**5: Build the code:**


If you generated Makefiles, run them like so:
```
make
```

If you generated Xcode project files, just open them in Xcode and build there.

Alternatively, you can also build them from the terminal by ```cd```ing into the directory of the tool you want to build (e.g. HedgeTools/HedgeOffsets) and entering the following command:
```
xcodebuild
```

**NOTE:** By default, this will compile a "Debug" build. To compile a "Release" build, simply run the following command instead:
```
xcodebuild -configuration Release
```

## Linux
**1: Install the required dependencies** (or equivalents for your distro of choice)**:**


Ubuntu/Debian-based distros:
```
sudo apt install git build-essential cmake libglm-dev liblz4-dev zlib1g-dev
```

Arch-based distros:
```
sudo pacman -S git gcc make cmake glm lz4 zlib
```

Void-based distros:
```
sudo xbps-install -S git gcc make cmake glm liblz4-devel zlib-devel
```

**2: Clone the HedgeLib repository:**
```
git clone https://github.com/Radfordhound/HedgeLib.git
```

**3: Go into the HedgeLib folder and switch to the HedgeLib++ branch:**
```
cd HedgeLib
git switch HedgeLib++
```

**4: Generate Makefiles with CMake:**
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../
```

**NOTE:** The generated Makefiles will produce a Debug build.
To change this, replace ```Debug``` in the above command with any of the following values:


Value           | Meaning
--------------- | -----------------------------------------------------------------------------
Debug           | Debug build; has debugging information (such as symbols) and no optimization.
Release         | Release build; is fully optimized and has no debugging information.
RelWithDebInfo  | Release build with debugging information.
MinSizeRel      | Release build, but optimized for size rather than speed.

**5: Run the Makefiles:**
```
make
```
