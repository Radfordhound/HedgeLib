project("HedgeLib")
	language("C++")
	cppdialect("C++17")
	targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
	
	-- Options
	newoption(
	{
		trigger = "type",
		value = "LibraryType",
		description = "What type of library to build HedgeLib as.",
		default = "static",
		allowed =
		{
			{ "static", "Build HedgeLib as a shared library (e.g. .DLL)." },
			{ "shared", "Build HedgeLib as a static library (e.g. .lib)." }
		}
	})
	
	if _OPTIONS["type"] == "static" then
		kind("StaticLib")
	else
		kind("SharedLib")
	end
	
	-- Platform-Specifics
	if target == "windows" then
		defines("NOMINMAX")
		buildoptions("/Zc:externConstexpr")
	end
	
	local includeDirs = { "include" }
	includedirs(includeDirs)
	files({ "include/**.h", "src/**.cpp", "src/**.h" })
	
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