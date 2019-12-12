project("HedgeEdit")
    language("C++")
    cppdialect("C++17")
    targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
    kind("WindowedApp")
    links("HedgeLib")
    runpathdirs("bin/%{cfg.platform}/%{cfg.buildcfg}")

    -- Backend Option
    newoption(
    {
        trigger = "backend",
        description = "Which backend HedgeEdit should use for rendering",
        default = "auto",
        allowed =
        {
            { "auto", "Pick automatically based on target platform" },
            { "d3d11", "Direct3D 11" },
            --{ "vk", "Vulkan" }
        }
    })

    -- Static or Shared
    if LibType == "shared" then
        defines("HL_DLL")
        postbuildcommands("copy /Y \"$(ProjectDir)License.txt\" \"$(TargetDir)License.txt\" >NUL")
        postbuildcommands("copy /Y \"$(SolutionDir)HedgeLib\\bin\\$(PlatformTarget)\\$(Configuration)\\HedgeLib.dll\" \"$(TargetDir)HedgeLib.dll\" >NUL")
    end

    -- Platform-Specifics
    if Target == "windows" then
        defines("NOMINMAX")
        files("app.manifest")

        -- Auto
        if _OPTIONS["backend"] == "auto" then
            _OPTIONS["backend"] = "d3d11"
        end

        -- Direct3D 11
        if _OPTIONS["backend"] == "d3d11" then
            files("shaders/**.hlsl")
            links({ "d3d11", "DXGI" })
            defines("D3D11")

            -- We rely on the fact that the qt module is already adding the obj dir to includedirs for us
            filter("files:**.hlsl")
                shadermodel("5.0")
                shaderentry("main")
                shadervariablename("dxc_%{file.basename}")
                shaderheaderfileoutput("$(IntDir)dxc_%{file.basename}.h")
                shaderobjectfileoutput("")

            filter("files:**_vs.hlsl")
                shadertype("Vertex")

            filter("files:**_ps.hlsl")
                shadertype("Pixel")

            filter({})
        -- Vulkan
        --elseif _OPTIONS["backend"] == "vk" then
            -- TODO
        else
            premake.error("Unknown backend.")
        end
    else
        pic "on"

        -- Auto
        if _OPTIONS["backend"] == "auto" then
            _OPTIONS["backend"] = "vk"
        end

        -- Direct3D 11
        if _OPTIONS["backend"] == "d3d11" then
            premake.error("Direct3D 11 backend is only supported on Windows.")
        -- Vulkan
        --elseif _OPTIONS["backend"] == "vk" then
            -- TODO
        else
            premake.error("Unknown backend.")
        end
    end
    
    -- Dependencies
    local deps = GetDepends("depends.lua")
    includedirs({ "ui", "../HedgeLib/include" })
    files({ "src/**.cpp", "src/**.h", "ui/**.cpp",
        "ui/**.h", "ui/**.ui", "ui/**.qrc" })
    
    -- MSC Optimization
    filter("toolset:msc")
        flags("MultiProcessorCompile")
    
    -- GCC C++ 17 Filesystem support
    filter("toolset:gcc or clang")
        links("stdc++fs")

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
        
    FinalizeQtDefault(deps.QtDir32, deps.QtDir64)
