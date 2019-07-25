project("HedgeEdit")
	language("C++")
	cppdialect("C++17")
	targetdir("bin/%{cfg.platform}/%{cfg.buildcfg}")
	kind("WindowedApp")
	links("HedgeLib")

    -- Static or Shared
    if LibType == "shared" then
        defines("HL_DLL")
        postbuildcommands("copy /Y \"$(ProjectDir)License.txt\" \"$(TargetDir)\License.txt\" >NUL")
        postbuildcommands("copy /Y \"$(SolutionDir)HedgeLib\\bin\\$(Platform)\\$(Configuration)\\HedgeLib.dll\" \"$(TargetDir)HedgeLib.dll\" >NUL")
    end
	
	-- Platform-Specifics
	if Target == "windows" then
		defines("NOMINMAX")
		files("app.manifest")
		links({ "d3d11", "DXGI" })
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
