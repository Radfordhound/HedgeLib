workspace("HedgeLib")
	configurations({ "Debug", "Release" })
	platforms({ "x86", "x64" })
	target = os.target()
	
	-- Define some functions
	function FixPath(path)
		local lastChar = string.sub(path, -1)
		if lastChar ~= "/" and lastChar ~= "\\" then
			return path .. "/"
		end
		
		return path
	end
	
	function LoadDependsLua(filePath)
		local deps = dofile(filePath)
		
		-- Ensure all paths end with /
		for k, v in pairs(deps) do
			deps[k] = FixPath(v)
		end
		
		return deps
	end
	
	-- Load some premake modules
	require("premake-modules/premake-qt/qt")
	dofile("qt.lua")
	qt = premake.extensions.qt
	
	-- Include Projects
	include("HedgeLib")
	
	if target == "windows" then
		include("HedgeEdit")
	end

	include("HedgeTools")