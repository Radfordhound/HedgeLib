project("HedgeOffsets")
    language("C++")
    cppdialect("C++17")
    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    kind("ConsoleApp")
    links("HedgeLib")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    
    includedirs({ "../../HedgeLib/include" })

    -- Static or Shared
    if LibType == "shared" then
        defines("HL_DLL")
        if Target == "windows" then
            postbuildcommands("copy /Y \"$(SolutionDir)HedgeLib\\bin\\$(PlatformTarget)\\$(Configuration)\\HedgeLib.dll\" \"$(TargetDir)HedgeLib.dll\" >NUL")
        end
    end
    
    -- Platform-Specifics
    if Target == "windows" then
        defines("NOMINMAX")
    end
    
    files({ "src/**.cpp", "src/**.h" })
    
    -- MSC Optimization
    filter("toolset:msc")
        flags("MultiProcessorCompile")

    -- Debug Configuration
    filter("configurations:Debug*")
        defines("DEBUG")
        symbols("On")

    -- Release Configuration
    filter("configurations:Release*")
        defines("NDEBUG")
        optimize("Speed")
        flags("LinkTimeOptimization")
        
    -- x86
    filter("platforms:x86")
        architecture("x86")
        defines("x86")
        
    -- x64
    filter("platforms:x64")
        architecture("x86_64")
        defines("x64")
