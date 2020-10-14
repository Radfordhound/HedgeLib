include("backends.lua")
include("../options.lua")

-- Generate list of allowed backends
local options =
{
    { "auto", "Choose automatically based on target platform (default)" }
}

for key, bk in pairs(Backends) do
    -- Generate description
    local desc = bk.Name
    if #bk.Platforms == 1 then
        desc = desc .. " (" .. GetFriendlyPlatformName(bk.Platforms[1]) .. " only)"
    end

    -- Add directory to backend info if necessary
    if bk.Dir == nil then
        bk.Dir = key
    end

    -- Add this option to the list
    table.insert(options, { key, desc })
end

-- Create premake option from said list
newoption(
{
    trigger = "backend",
    description = "Which backend to use",
    default = "auto",
    allowed = options
})

-- Get helper variables
if _OPTIONS["backend"] == "auto" or _OPTIONS["backend"] == nil then
    Backend = DefaultBackends[Target]
    if Backend == nil then
        premake.error("Unknown or unsupported target platform")
    end
else
    Backend = Backends[_OPTIONS["backend"]]
    if Backend == nil then
        premake.error("Unknown or unsupported backend")
    end
end

if not table.contains(Backend.Platforms, Target) then
    premake.error(Backend.Name .. " is not supported on " ..
        GetFriendlyPlatformName(Target))
end
