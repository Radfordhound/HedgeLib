project("HedgeArcPack")
	language("C++")
	cppdialect("C++17")
	targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
	kind("ConsoleApp")
	links("HedgeLib")
	
	includedirs({ "../../HedgeLib/include", "include" })

    -- Static or Shared
    if LibType == "shared" then
        defines("HEDGELIB_DLL")
    end
	
	-- Platform-Specifics
	if Target == "windows" then
		defines("NOMINMAX")
	end
	
	files({ "src/**.cpp", "src/**.h", "include/**.h" })
	
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
