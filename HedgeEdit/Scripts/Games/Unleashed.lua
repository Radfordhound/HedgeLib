function LoadTexturesInDir(dir, loadingTxt)
	if not IODirExists(dir) then
		return
	end

	local files = IOGetFilesInDir(
		dir, "*.dds", false)

	if files ~= nil and #files > 0 then
		UIShowProgress()

		for i = 1, #files do
			UIChangeProgress(((i - 1) / #files) * 100)
			UIChangeLoadStatus(string.format(
				loadingTxt .. " %02d/%02d", i, #files))

			LoadTexture(files[i], IOGetNameWithoutExtension(files[i]))
		end

		UIHideProgress()
	end
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Unleashed")

	-- Get Day/Night Flag
	local dayNightFlag = string.sub(stageID, 4, 4)
	if dayNightFlag ~= 'D' and dayNightFlag ~= 'N' then
		LogError("ERROR: Stage ID did not begin with \"ActD\" or \"ActN\"." ..
			"Please enter a valid stage ID.")
		return
	end

	-- Get Country
	local suffix = string.sub(stageID, 6)
	local country = nil
	local countryList =
	{
		"Africa",
		"Beach",
		"China",
		"EU",
		"Mykonos",
		"NY",
		"Petra",
		"Snow"
	}

	for i, c in ipairs(countryList) do
		if string.find(suffix, c) then
			country = c
			break
		end
	end

	-- #AR (E.G. #ActD_MykonosAct1)
	UIChangeStatus("Extracting #" .. stageID .. "...")
	Extract("{0}/#{1}.arl", "{0}/#{1}", "#AR")

	-- Set Data
	local files = IOGetFilesInDir("{0}/#{1}", "*.set.xml", false)
	if files ~= nil and #files > 0 then
		UIShowProgress()

		for i = 1, #files do
			UIChangeProgress(((i - 1) / #files) * 100)
			UIChangeLoadStatus(string.format(
				"Set Data %02d/%02d", i, #files))

			LoadSetLayer(files[i])
		end

		UIHideProgress()
	end

	UIToggleSetsSaving(true)

	-- AR (E.G. ActD_MykonosAct1)
	UIChangeStatus("Extracting " .. stageID .. "...")
	Extract("{0}/{1}.arl", "{0}/{1}", "AR")

	-- Stage PFD (E.G. Packed/Stage)
	UIChangeStatus("Extracting Stage PFD...")
	Extract("{0}/Packed/{1}/Stage.pfd", "{0}/Packed/Stage", "StagePFD")

	-- Stage-Add PFD (E.G. Packed/Stage-Add)
	UIChangeStatus("Extracting Stage-Add PFD...")
	Extract("{0}/Packed/{1}/Stage-Add.pfd", "{0}/Packed/Stage-Add", "StageAddPFD")

	-- TODO: Decompress AR files within PFD

	if country ~= nil then
		-- Common
		UIChangeStatus("Extracting Common AR...")
		local cmnName = ("{0}/Cmn" .. country)

		Extract(cmnName .. ".arl", cmnName, "Cmn")

		-- Common Textures
		LoadTexturesInDir(cmnName, "Common Textures")

		-- Common Terrain AR
		UIChangeStatus("Extracting Common Terrain AR...")
		local cmnTerrainName = ("{0}/CmnAct" .. dayNightFlag ..
			"_Terrain_" .. country)

		Extract(cmnTerrainName .. ".arl", cmnTerrainName, "CmnTerrain")

		-- Common Terrain Textures
		LoadTexturesInDir(cmnTerrainName, "Common Terrain Textures")
	end

	-- Terrain
	LoadTerrainList("{0}/#{1}/terrain.terrain",
		"{0}/Packed/Stage", "{0}/{1}")
	-- TODO
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data
	-- TODO: Finish GensSetData Save method then uncomment this line:
	--SaveSetLayers("{0}/#{1}", "", ".xml", true)

	--[[
	UIChangeStatus("Repacking #" .. stageID .. "...")
	SetDataType("Gens")
	Repack("{0}/#{1}.ar", "{0}/#{1}", "#AR", false, true)
	--]]
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end