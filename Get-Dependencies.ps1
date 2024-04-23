param(
    # The directory to install the dependencies into
    $Architecture = ('Win32', 'x64')[[Environment]::Is64BitOperatingSystem],
    $DependenciesDirectory = "Dependencies/$Architecture"
)

# Create dependencies directory
New-Item $DependenciesDirectory -ItemType Directory -Force > $null

# Define functions for later use
function Install-CMakeProject {
    param(
        $Name,
        $Url,
        $SourcePath,
        $Arguments,
        $BuildDebugConfig=$false,
        $OutDirectory=$DependenciesDirectory
    )

    Write-Host "Downloading $Name..." -ForegroundColor Green

    # Create temporary download/build directory
    $TmpBuildDirectory = $(Join-Path -Path $OutDirectory -ChildPath "tmp")
    New-Item -Path $TmpBuildDirectory -ItemType Directory -Force > $null

    # Get project source/build folders
    $ProjectSourceFolder = $(Join-Path -Path $TmpBuildDirectory -ChildPath "src")
    $ProjectBuildFolder = $(Join-Path -Path $TmpBuildDirectory -ChildPath "build")

    # Download project source archive
    $ArchiveFile = $(Join-Path -Path $TmpBuildDirectory -ChildPath $(Split-Path -Path $Url -Leaf))
    Invoke-WebRequest -Uri $Url -OutFile $ArchiveFile
    Write-Host "Extracting $Name..." -ForegroundColor Green

    # Extract project source archive
    Expand-Archive -Path $ArchiveFile -DestinationPath $ProjectSourceFolder -Force
    Write-Host "Generating CMake files for $Name..." -ForegroundColor Green

    # Delete project source archive
    Remove-Item -Path $ArchiveFile -Force

    # Create project build directory
    New-Item -Path $ProjectBuildFolder -ItemType Directory -Force > $null

    # Build and install project
    $AbsoluteOutDirectory = $(Resolve-Path -Path $OutDirectory)
    $InstallPrefixArgument = "-DCMAKE_INSTALL_PREFIX=$AbsoluteOutDirectory"
    Push-Location $ProjectBuildFolder

    try {
        # Generate CMake files for project
        & cmake -Wno-deprecated -A $Architecture ../src/$SourcePath $Arguments $InstallPrefixArgument
        if (!$?) {
            throw "Failed to generate files for CMake project $Name"
        }

        # Build project via the generated CMake files
        if ($BuildDebugConfig) {
            Write-Host "Building $Name in Debug configuration..." -ForegroundColor Green
            & cmake --build . --config Debug

            if (!$?) {
                throw "Failed to build CMake project $Name in Debug configuration"
            }
        }

        Write-Host "Building $Name in Release configuration..." -ForegroundColor Green
        & cmake --build . --config Release

        if (!$?) {
            throw "Failed to build CMake project $Name in Release configuration"
        }

        # Install project via the generated CMake files
        if ($BuildDebugConfig) {
            Write-Host "Installing $Name in Debug configuration..." -ForegroundColor Green
            & cmake --install . --config Debug

            if (!$?) {
                throw "Failed to install CMake project $Name in Debug configuration"
            }
        }

        Write-Host "Installing $Name in Release configuration..." -ForegroundColor Green
        & cmake --install . --config Release

        if (!$?) {
            throw "Failed to install CMake project $Name in Release configuration"
        }
    }
    finally {
        Pop-Location
    }

    # Delete project build Directory
    Remove-Item -Path $TmpBuildDirectory -Recurse -Force
}

# Download and install all required dependencies
$Dependencies = @()
$Dependencies += { Install-CMakeProject -Name "robin_hood" `
    -Url https://github.com/martinus/robin-hood-hashing/archive/refs/heads/master.zip `
    -SourcePath "robin-hood-hashing-master" `
    -Arguments "-DRH_STANDALONE_PROJECT=OFF"
}

$Dependencies += { Install-CMakeProject -Name "glm" `
    -Url https://github.com/g-truc/glm/archive/refs/heads/master.zip `
    -SourcePath "glm-master"
}

$Dependencies += { Install-CMakeProject -Name "lz4" `
    -Url https://github.com/lz4/lz4/archive/refs/heads/dev.zip `
    -SourcePath "lz4-dev/build/cmake" `
    -Arguments "-DLZ4_BUILD_CLI=OFF", "-DLZ4_BUILD_LEGACY_LZ4C=OFF", `
        "-DBUILD_SHARED_LIBS=OFF", "-DBUILD_STATIC_LIBS=ON"
}

$Dependencies += { Install-CMakeProject -Name "zlib" `
    -Url https://github.com/madler/zlib/archive/refs/heads/master.zip `
    -SourcePath "zlib-master" `
    -BuildDebugConfig $true

    # HACK: Get rid of shared zlib libraries so FindZLIB will link it statically on Windows
    Remove-Item -Path $(Join-Path -Path $DependenciesDirectory -ChildPath "bin") -Recurse -Force
    Remove-Item -Path $(Join-Path -Path $DependenciesDirectory -ChildPath "lib\zlibd.lib") -Force
    Remove-Item -Path $(Join-Path -Path $DependenciesDirectory -ChildPath "lib\zlib.lib") -Force
}

$Dependencies += { Install-CMakeProject -Name "glfw3" `
    -Url https://github.com/glfw/glfw/archive/refs/heads/master.zip `
    -SourcePath "glfw-master" `
    -Arguments "-DGLFW_BUILD_EXAMPLES=OFF", "-DGLFW_BUILD_TESTS=OFF"
}

$Dependencies += { Install-CMakeProject -Name "RapidJSON" `
    -Url https://github.com/Tencent/rapidjson/archive/refs/heads/master.zip `
    -SourcePath "rapidjson-master" `
    -Arguments "-DRAPIDJSON_BUILD_DOC=OFF", "-DRAPIDJSON_BUILD_EXAMPLES=OFF", `
        "-DRAPIDJSON_BUILD_TESTS=OFF", "-DRAPIDJSON_HAS_STDSTRING=ON"
}

for ($i = 0; $i -lt $Dependencies.Length; $i++) {
    Invoke-Command $Dependencies[$i]
}

Write-Host "Finished downloading and installing all required dependencies!" -ForegroundColor Green
