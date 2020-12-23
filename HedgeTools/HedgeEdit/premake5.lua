include("../../depends/imgui")

project("HedgeEdit")
    language("C")
    kind("WindowedApp")
    
    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    includedirs({
        "../../HedgeLib/include",
        "../../HedgeRender/include",
        "../../depends",
        "../../depends/cglm/include"
    })

    -- Link to HedgeLib and HedgeRender
    if LibType == "shared" then
        defines({ "HL_IS_DLL", "HR_IS_DLL" })
    end

    links("cglm")
    dofile("../../HedgeRender/link.lua")
    
    -- Platform-Specifics
    if Target == "windows" then
        defines("NOMINMAX")
        dpiawareness("High")
    end
    
    files({ "src/**.c", "src/**.h" })
    
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
