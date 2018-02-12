function ExtractResources(sourceDir, destDir)
	SetDataType("S06")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("S06")
	local dir, type = dataDir, "xenon"

	-- TODO: Make an IOPathCombine function please you beautiful moron
	if IODirExists(dir .. "/xenon") then
		type = "xenon"
	elseif IODirExists(dir .. "/ps3") then
		type = "ps3"
	else
		LogError("ERROR: Invalid Data Directory")
		return
	end

	dir = dir .. "/" .. type

	-- Set Data (E.G. xenon/archives/scripts.arc)
	UIChangeStatus("Extracting Scripts ARC...")
	Extract(dir .. "/archives/scripts.arc", "{0}/scripts", "Scripts")

	local setDir = "{0}/scripts/" .. type .. "/placement/{1}"
	local files = IOGetFilesInDir(setDir, "*.set", false)
	
	if files ~= nil and #files > 0 then
		UIShowProgress()

		for i = 1, #files do
			UIChangeProgress(((i - 1) / #files) * 100)
			UIChangeLoadStatus(string.format(
				"Set Data %02d/%02d", i, #files))

			LoadSetLayer(files[i]) -- TODO: Load Object Models
		end

		UIHideProgress()
	end

	UIToggleSetsSaving(true)
	-- TODO: Finish This
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data (E.G. xenon/archives/scripts.arc)
	SetDataType("S06")

	local dir, type = cacheDir .. "/scripts", "xenon"
	if IODirExists(dir .. "/xenon") then
		type = "xenon"
	elseif IODirExists(dir .. "/ps3") then
		type = "ps3"
	else
		LogError("ERROR: Invalid Data Directory")
		return
	end

	dir = dir .. "/" .. type

	local setDir = "{0}/scripts/" .. type .. "/placement/{1}"
	SaveSetLayers(setDir, "", ".set", true)
	
	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end