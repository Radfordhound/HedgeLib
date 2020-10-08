project("HedgeLib")
    language("C")

    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    
    includedirs({ "include" })
    files({ "include/**.h", "src/**.c", "src/**.h" })

    -- Options
    if LibType == "static" then
        kind("StaticLib")
    else
        kind("SharedLib")
        defines({ "HL_IS_DLL", "HL_IS_BUILDING_DLL" })
    end
    
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
