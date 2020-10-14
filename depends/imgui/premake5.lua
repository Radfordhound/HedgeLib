project("imgui")
    language("C++")

    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    files({ "**.cpp", "**.h" })
    kind("StaticLib")
    
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
