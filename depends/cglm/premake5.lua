project("cglm")
    language("C")

    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    
    includedirs({ "include", "../../HedgeLib/include" })
    files({ "include/**.h", "src/**.c", "src/**.h" })

    -- Options
    if LibType == "static" then
        kind("StaticLib")
        defines({ "CGLM_STATIC" })
    else
        kind("SharedLib")
        defines({ "CGLM_EXPORTS" })
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
