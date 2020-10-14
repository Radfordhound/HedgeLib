-- Backends list
-- (Feel free to add to this!)
Backends =
{
    d3d11 =
    {
        Name = "Direct3D 11",
        CPPDialect = "C++11",
        Platforms = { "windows" },
        CustomBuildFunc = function(self)
            includedirs("%{cfg.objdir}")
            files("backends/" .. self.Dir .. "/shaders/**.hlsl")
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
        end,

        LinkFunc = function()
            print("Linking d3d11...") -- TODO: Remove this
            links({ "d3d11", "dxgi", "dxguid" })
            links("d3dcompiler")  -- TODO: Remove this
            filter({})
        end
    }
}

-- Default backends list
-- (Feel free to add to this!)
DefaultBackends =
{
    windows = Backends.d3d11
}

-- Friendly platform name list
-- (Feel free to add to this! You may also have to create a premake extension for your target platform)
FriendlyPlatformNames =
{
    -- Desktop
    windows = "Windows",
    macosx = "macOS",
    linux = "Linux",
    aix = "AIX",
    bsd = "OpenBSD/NetBSD/FreeBSD",
    haiku = "Haiku",
    hurd = "GNU/Hurd",
    solaris = "Solaris",

    -- Mobile
    ios = "iOS",
    android = "Android"
}

-- Helper functions
function GetFriendlyPlatformName(platform)
    local friendlyName = FriendlyPlatformNames[platform]
    if friendlyName == nil then
        friendlyName = platform
    end

    return friendlyName
end
