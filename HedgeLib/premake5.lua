project("HedgeLib")
    language("C")

    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    
    includedirs({ "include", "../depends/cglm/include" })
    files({ "include/**.h", "src/**.c", "src/**.h" })
    links("cglm")

    -- Options
    if LibType == "static" then
        kind("StaticLib")
    else
        kind("SharedLib")
        defines({ "HL_IS_DLL", "HL_IS_BUILDING_DLL" })
    end

    -- macOS zlib fix
    -- TODO: We're supposed to use zlib's configure script but
    -- djophgdnzoANDgoSDPGSGNs;JIKL DNGjkSPDGnS it's a pain so we'll just do this for now
    filter("system:macosx")
        defines("HAVE_UNISTD_H")
    
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
