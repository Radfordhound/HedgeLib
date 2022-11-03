![HedgeLib Logo](Logo-small.png?raw=true)

*(Thanks to [Gotta Play Fast](https://www.youtube.com/channel/UCZfOGBkXRKICFozWU5bE0Xg) for the wonderful project logo!)*

**IMPORTANT:** All code committed to this repository, preferably, should follow [these style guidelines](https://github.com/Radfordhound/HedgeLib/wiki/Code-Style).

## Download

This repository uses [AppVeyor](https://www.appveyor.com) to automatically build every commit!
As such, you don't have to manually build any of the tools/libraries in this repository if you simply want to try out the latest versions.

- [Stable (release) builds](https://github.com/Radfordhound/HedgeLib/releases)
- [Latest (development) builds](https://ci.appveyor.com/project/Radfordhound/hedgelib)

## HedgeLib
HedgeLib is C++17 library [under the MIT license](LICENSE.txt) that aims to make modding games in the Sonic the Hedgehog franchise easier, similar to [Dario's excellent "LibGens"](https://github.com/DarioSamo/libgens-sonicglvl).

Right now, HedgeLib supports the following:

### Hedgehog Engine 1/2
- "PACx" Packfiles (.pac; v403/v402/v3/v2)
- Terrain instance infos (.terrain-instanceinfo; v0/v5)
- Skeletal Models (.model; v2/v4/v5)
- Terrain Models (.terrain-model; v5)
- Materials (.material; v1/v3)
- Texsets (.texset; v0)
- Texture entries (.texture; v1)
- "Mirage" headers (used in models/materials from Lost World and beyond)

### Ninja Next ("Sega NN")
- Node Motions (Skeletal Animations) [.xnm, .snm, .gnm, .cnm, .inm, .enm, .ynm, .znm]
- Camera Motions [.xnc, .snc, .gnc, .cnc, .inc, .enc, .ync, .znc]
- Light Motions [.xnl, .snl, .gnl, .cnl, .inl, .enl, .ynl, .znl]
- Morph Motions [.xnf, .snf, .gnf, .cnf, .inf, .enf, .ynf, .znf]
- Material Motions [.xnv, .snv, .gnv, .cnv, .inv, .env, .ynv, .znv]

### Other
- "BINA" binary container format (v1/v2)
- "AR" Archives (.ar/.pfd)
- Packed File Indexes (.pfi)

## HedgeRender
A "simple" C++17 graphics/rendering engine designed around Vulkan.
Some of the features include:

- A simple render graph system that allows for specifying render passes/subpasses in a simple yet powerful manner.
- An "upload batch" system that allows for fast and easy batching of resource uploads to the GPU.

## HedgeTools
A collection of miscellaneous tools that utilize HedgeLib.

- [HedgeEdit](HedgeTools/HedgeEdit)
  * A C++17 all-in-one ("AIO") editor built on top of HedgeLib and HedgeRender that aims to support quick and powerful editing of levels/resources from all games HedgeLib supports.
- [HedgeArcPack](HedgeTools/HedgeArcPack)
  * A C++17 Command-Line tool that allows quick and powerful extraction and generation of archiving formats from all games HedgeLib supports.
- [HedgeOffsets](HedgeTools/HedgeOffsets)
  * A C++17 Command-Line tool that reads the offset table in BINA/Hedgehog Engine formats and prints each offset. Useful for debugging and reverse-engineering of undocumented formats which contain an offset table.

# Manually Building
Building HedgeLib is supposed to be easy.
If you're having trouble building with the following instructions, please [create an issue](https://github.com/Radfordhound/HedgeLib/issues/new).

## Windows (Visual Studio)
### 1: Install the following prerequisites (if you haven't already)
- [CMake](https://cmake.org/download/)
- [git](https://git-scm.com/download/win)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (**NOTE:** Only required for HedgeRender/HedgeEdit)

**IMPORTANT:** Be sure to select "Add CMake to the system PATH for all users" during the CMake installation, or you'll encounter an error while following these steps!

### 2: Open a PowerShell window and clone the HedgeLib repository
```ps
git clone https://github.com/Radfordhound/HedgeLib.git
cd HedgeLib
```

### 3: Download dependencies using the included Powershell script
```ps
.\Get-Dependencies.ps1
```

### 4: Generate a Visual Studio Solution (.sln file) with CMake
```ps
cmake -S . -B build
```

**OPTIONAL:** Use the -A flag to specify a specific architecture to target:

Value   | Architecture
------- | -------------------------
Win32   | 32-bit x86 Architecture
x64     | 64-bit x86 Architecture
ARM     | 32-bit ARM Architecture
ARM64   | 64-bit ARM Architecture

**OPTIONAL:** Use the -G flag to specify a specific Visual Studio version to target:

Value                     | Visual Studio Version
------------------------- | -----------------------------------------
Visual Studio 17 2022     | Visual Studio 2022
Visual Studio 16 2019     | Visual Studio 2019
Visual Studio 15 2017     | Visual Studio 2017
Visual Studio 14 2015     | Visual Studio 2015


### 5: Open the resulting HedgeLib.sln file (contained within the "build" directory) in Visual Studio and build
Done!

## macOS
### 1: Install the following prerequisites (if you haven't already)
- [Homebrew](https://brew.sh/)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#mac) (**NOTE:** Only required for HedgeRender/HedgeEdit)

### 2: Open a terminal window and install the required dependencies

Install all pre-packaged dependencies:

```sh
brew install cmake git glm lz4 zlib glfw
```

Build/install robin-hood-hashing:

```sh
git clone https://github.com/martinus/robin-hood-hashing.git
cd robin-hood-hashing
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRH_STANDALONE_PROJECT=OFF
cmake --build build --config Release
sudo cmake --install build --config Release
cd ../
```

### 3: Clone the HedgeLib repository
```sh
git clone https://github.com/Radfordhound/HedgeLib.git
cd HedgeLib
```

### 4: Generate Makefiles or Xcode project files with CMake

You can build using Makefiles, or using an Xcode project.

- Makefiles are simple and don't require Xcode to be installed.
- Xcode is Xcode.

Pick whichever one you prefer.

#### To generate Makefiles
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

**OPTIONAL:** Replace ```Debug``` in the above command with ```Release``` to generate Release build Makefiles.

#### To generate an Xcode project
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Xcode"
```

**NOTE:** If you get this weird error:

```
CMake Error: Xcode 1.5 not supported.
```

It means CMake isn't finding the correct Xcode installation. To fix it, run the following commands, but with "/Applications/Xcode_10.1.app" replaced with the path to your machine's Xcode.app file:

```sh
sudo /usr/bin/xcode-select --switch /Applications/Xcode_10.1.app
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Xcode"
```

### 5: Build the code

**If you generated Makefiles**, execute the following command to build:

```sh
cmake --build build
```

**If you generated an Xcode project**, just open the resulting project (contained within the "build" directory) in Xcode and build

Done!

## Linux
### 1: Install the required dependencies (or equivalents)

#### Ubuntu/Debian

Install Vulkan SDK Repository:

```sh
wget -qO - https://gist.githubusercontent.com/Radfordhound/6e6ce00535d14ae87d606ece93f1e336/raw/9796f644bdedaa174ed580a8aa6874ab82853170/install-lunarg-ubuntu-repo.sh | sh
```

Install all pre-packaged dependencies:

```sh
sudo apt install git build-essential cmake libglm-dev liblz4-dev zlib1g-dev libglfw3-dev vulkan-sdk
```

Build/install robin-hood-hashing:

```sh
git clone https://github.com/martinus/robin-hood-hashing.git
cd robin-hood-hashing
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRH_STANDALONE_PROJECT=OFF
cmake --build build --config Release
sudo cmake --install build --config Release
cd ../
```

#### Arch
```sh
sudo pacman -S git gcc make cmake glm lz4 zlib glfw-wayland vulkan-headers vulkan-validation-layers shaderc
yay -S robin-hood-hashing
```

#### Void

Install all pre-packaged dependencies:

```sh
sudo xbps-install -S git gcc make cmake glm liblz4-devel zlib-devel glfw-devel Vulkan-Headers Vulkan-ValidationLayers shaderc
```

Build/install Robin-hood-hashing:

```sh
git clone https://github.com/martinus/robin-hood-hashing.git
cd robin-hood-hashing
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRH_STANDALONE_PROJECT=OFF
cmake --build build --config Release
sudo cmake --install build --config Release
cd ../
```

### 2: Clone the HedgeLib repository
```sh
git clone https://github.com/Radfordhound/HedgeLib.git
cd HedgeLib
```

### 3: Generate Makefiles with CMake
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

**OPTIONAL:** Replace ```Debug``` in the above command with ```Release``` to generate Release build Makefiles.

### 4: Build the code with CMake
```sh
cmake --build build
```

Done!
