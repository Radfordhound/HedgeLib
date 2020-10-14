include("options.lua")
include("../depends/imgui")

local hedgeRenderKind = "StaticLib"
if LibType == "shared" then
    hedgeRenderKind = "SharedLib"
end

-- Create HedgeRender project
project("HedgeRender")
    language("C++")
    cppdialect("C++11")
    kind(hedgeRenderKind)

    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    
    includedirs({ "include", "../HedgeLib/include", "../depends", "depends" })
    files({
        "include/**.h", "src/**.cpp", "src/**.h",
        "depends/**.h", "depends/**.cpp", "depends/**.c"
    })

    -- Options
    if LibType == "shared" then
        defines({ "HR_IS_DLL", "HR_IS_BUILDING_DLL" })
        links({ "HedgeLib", "imgui" })

        if Target == "windows" then
            -- TODO: Copy dll to correct location
        end
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

-- Create backend project
project("HedgeRender_" .. Backend.Dir)
    -- Set language
    language("C++")

    if Backend.CPPDialect then
        cppdialect(Backend.CPPDialect)
    end

    kind(hedgeRenderKind)
    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")
    includedirs({ "include", "../HedgeLib/include", "../depends" })

    files(
    {
        "backends/" .. Backend.Dir .. "/**.cpp",
        "backends/" .. Backend.Dir .. "/**.h"
    })

    if Backend.CustomBuildFunc then
        Backend:CustomBuildFunc()
    end

    -- Link to backends if compiling as a shared library
    if LibType == "shared" then
        defines({ "HR_IS_DLL", "HR_BACKEND_IS_DLL", "HR_IS_BUILDING_BACKEND_DLL" })
        if Target == "windows" then
            -- TODO: Copy dll to correct location
        end
        
        -- Link dependencies
        DisableBackend = true
        dofile("link.lua")
        DisableBackend = nil
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
