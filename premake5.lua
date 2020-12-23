workspace("HedgeLib")
    configurations({ "Debug", "Release" })
    platforms({ "x86", "x64" })

    -- Get some variables
    Host = os.host()
    Target = os.target()

    -- Define some functions
    function JoinPaths(...)
        local pth = path.join(...)
        if Target == "windows" then
            pth = path.translate(pth, "\\")
        end

        return pth
    end
    
    -- Setup premake options
    include("options.lua")

    -- Disable some warnings on windows
    if Target == "windows" then
        disablewarnings("4996") -- please use wcscpy_s even though it's not in the C++ standard we're begging you
    end

    -- Include dependency projects
    include("depends/cglm")

    -- Include projects
    include("HedgeLib")

    -- TODO: Also support non-Windows platforms
    if Target == "windows" then
        include("HedgeRender")
    end

    include("HedgeTest")
    include("HedgeTools")
