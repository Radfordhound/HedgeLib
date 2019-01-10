project("HedgeEdit")
	language("C++")
	cppdialect("C++17")
	targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
	kind("WindowedApp")
	links("HedgeLib")
	
	-- Platform-Specifics
	if target == "windows" then
		defines("NOMINMAX")

		-- So implementing alternate graphics APIs is easier if we decide to later on.
		defines("D3D")
		links({ "d3d11", "DXGI" })
	end
	
	-- Dependencies
	local deps = LoadDependsLua("depends.lua")
	includedirs({ "ui", "../HedgeLib/include" })
	files({ "src/**.cpp", "src/**.h", "ui/**.cpp",
		"ui/**.h", "ui/**.ui", "ui/**.qrc" })
		
	-- TODO: Is app.manifest needed anymore? (Qt docs actually mentioned it)
	
	-- MSC Optimization
	filter("toolset:msc")
		flags("MultiProcessorCompile")
	
	-- GCC C++ 17 Filesystem support
	filter("toolset:gcc")
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

	-- Copy License
	filter({ "configurations:Debug*", "platforms:x86" })
		postbuildcommands("copy /y License.txt bin\\x86\\Debug\\License.txt >NUL")

	filter({ "configurations:Debug*", "platforms:x64" })
		postbuildcommands("copy /y License.txt bin\\x64\\Debug\\License.txt >NUL")

	filter({ "configurations:Release*", "platforms:x86" })
		postbuildcommands("copy /y License.txt bin\\x86\\Release\\License.txt >NUL")

	filter({ "configurations:Release*", "platforms:x64" })
		postbuildcommands("copy /y License.txt bin\\x64\\Release\\License.txt >NUL")