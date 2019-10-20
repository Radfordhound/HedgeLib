workspace("HedgeLib")
	configurations({ "Debug", "Release" })
	platforms({ "x86", "x64" })

    -- Get some variables
	Target = os.target()

	-- Define some functions
    function JoinPaths(...)
        local pth = path.join(...)
        if Target == "windows" then
            pth = path.translate(pth, "\\")
        end

        return pth
    end

	function GetDepends(filePath)
        -- Load depends lua file if it exists
        if filePath == nil then
            filePath = "depends.lua"
        end

        if os.isfile("depends.lua") then
		    return dofile("depends.lua")
        end

        -- Otherwise, ask user where Qt is installed
        local deps = {}
        print("Qt installation directory could not be auto-determined.")

        print("Please enter the location of your 32-bit Qt installation (e.g. C:\\Qt\\5.12.0\\msvc2017):")
        deps.QtDir32 = io.read()

        print("Please enter the location of your 64-bit Qt installation (e.g. C:\\Qt\\5.12.0\\msvc2017_64):")
        deps.QtDir64 = io.read()

        print("Generating depends.lua so Qt installation directory can be remembered in the future...")
        local file = io.open("depends.lua", "w")
        file:write("return {\n")
        file:write("\tQtDir32 = \"" .. string.gsub(deps.QtDir32, "\\", "\\\\") .. "\",\n")
        file:write("\tQtDir64 = \"" .. string.gsub(deps.QtDir64, "\\", "\\\\") .. "\"\n")
        file:write("}\n")
        file:close()

        return deps
	end
	
	-- Load some premake modules
	require("premake-modules/premake-qt/qt")
	dofile("qt.lua")
	qt = premake.extensions.qt
	
    -- Get some options
    include("options.lua")

    -- Disable some warnings on windows
    if Target == "windows" then
        disablewarnings("4996") -- please use wcscpy_s even though it's not in the C++ standard we're begging you
    end

	-- Include Projects
	include("HedgeLib")
	
	if Target == "windows" then
		include("HedgeEdit")
	end

	include("HedgeTools")
