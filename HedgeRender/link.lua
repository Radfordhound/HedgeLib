-- Include options.lua
include("options.lua")

-- Link HedgeLib, HedgeRender, and imgui
links("HedgeLib")
links("HedgeRender")
links("imgui")

-- Windows-specific options
if Target == "windows" then
    -- Use widechar entry point
    entrypoint("wWinMainCRTStartup")
end

-- Link HedgeRender backend
if not DisableBackend then
    links("HedgeRender_" .. Backend.Dir)
end

-- Link backend dependencies
Backend.LinkFunc()
