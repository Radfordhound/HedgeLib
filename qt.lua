local modulesQtPrefix = "Qt5"
local modulesQt = { "core", "gui", "widgets" }
local platformsQt = { "qminimal", "qwindows" }

function SetModulesPrefix(v)
	modulesQtPrefix = v
end

function SetModules(v)
	modulesQt = v
end

function SetPlatforms(v)
	platformsQt = v
end

local function GetFilter(v)
	if v.Filter == nil then
		return v.Dir
	else
		return v.Filter
	end
end

function FinalizeQt(architectures, configs, binDir)
	filter({})
	qt.enable()
	qtmodules(modulesQt)
	qtprefix(modulesQtPrefix)

	if architectures == nil then
		return
	end

	-- Default configs
	if configs == nil then
		configs =
		{
			{
				Filter = "Debug*",
				Dir = "Debug",
				Suffix = "d"
			},
			{
				Filter = "Release*",
				Dir = "Release"
			}
		}
	end
	
	-- Default bin dir
	if binDir == nil then
		binDir = "bin/"
	else
		binDir = FixPath(binDir)
	end
	
	-- Set configuration filters
	for ck, cv in pairs(configs) do
		if cv.Suffix ~= nil then
			filter("configurations:" .. GetFilter(cv))
			qtsuffix(cv.Suffix)
		end
	end
	
	-- Set platform filters
	for af, av in pairs(architectures) do
		filter("platforms:" .. GetFilter(av))
		qtpath(av.QtDir)
	end
	
	-- Set configuration & platform filters
	for ck, cv in pairs(configs) do
		for af, av in pairs(architectures) do
			filter(
			{
				"configurations:" .. GetFilter(cv),
				"platforms:" .. GetFilter(av)
			})
			
			-- Get project directory
			local pDir = FixPath(binDir ..
				FixPath(av.Dir) .. FixPath(cv.Dir))
			
			-- Get suffix
			local suffixQt = ""
			if cv.Suffix ~= nil then
				suffixQt = cv.Suffix
			end
			
			-- Set post-build commands
			if target == "windows" then
				-- Copy Qt Modules
				local cmds = { "@echo off", "echo Copying Qt Modules..." }
				for k, v in pairs(modulesQt) do
					local name = modulesQtPrefix .. qt.modules[v].name .. suffixQt
					local srcDst =
						av.QtDir .. "bin/" .. name .. ".dll\" \"" ..
						pDir .. name .. ".dll\" >NUL"
						
					srcDst = string.gsub(srcDst, "/", "\\")
					cmds[#cmds + 1] = "copy /y \"" .. srcDst
				end
				
				-- Copy Qt Platform DLLs
				cmds[#cmds + 1] = "echo Copying Qt Platform DLLs..."
				cmds[#cmds + 1] = "if not exist \"" .. pDir ..
					"platforms\" mkdir \"" .. pDir .. "platforms\""
				
				for k, v in pairs(platformsQt) do
					local name = v .. suffixQt
					local srcDst =
						av.QtDir .. "plugins/platforms/" .. name .. ".dll\" \"" ..
						pDir .. "platforms/" .. name .. ".dll\" >NUL"
						
					srcDst = string.gsub(srcDst, "/", "\\")
					cmds[#cmds + 1] = "copy /y \"" .. srcDst
				end
			
				links(av.QtDir .. "lib/qtmain" .. suffixQt)
				postbuildcommands(cmds)
			else
				-- TODO
			end
		end
	end
end

-- Finalizes Qt using default architectures/configurations/bin dir
function FinalizeQtDefault(x86Dir, x64Dir)
	FinalizeQt(
	{
		-- Architectures
		{
			Dir = "x86",
			QtDir = x86Dir
		},
		{
			Dir = "x64",
			QtDir = x64Dir
		}
	})
end