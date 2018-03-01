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

	-- AR (E.G. ActD_MykonosAct1)
	UIChangeStatus("Extracting " .. stageID .. "...")
	AddResourceDirectory("{0}/{1}")
	Extract("{0}/{1}.arl", "{0}/{1}", "AR")

	-- Common
	if country ~= nil then
		UIChangeStatus("Extracting Common AR...")
		local cmnName = ("{0}/Cmn" .. country)
		AddResourceDirectory(cmnName)

		Extract(cmnName .. ".arl", cmnName, "Cmn")

		-- Common Terrain AR
		UIChangeStatus("Extracting Common Terrain AR...")
		local cmnTerrainName = ("{0}/CmnAct" .. dayNightFlag ..
			"_Terrain_" .. country)

		AddResourceDirectory(cmnTerrainName)
		Extract(cmnTerrainName .. ".arl", cmnTerrainName, "CmnTerrain")
	end

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

	-- Stage PFD (E.G. Packed/Stage)
	UIChangeStatus("Extracting Stage PFD...")
	Extract("{0}/Packed/{1}/Stage.pfd", "{0}/Packed/Stage", "StagePFD")

	-- Stage-Add PFD (E.G. Packed/Stage-Add)
	UIChangeStatus("Extracting Stage-Add PFD...")
	Extract("{0}/Packed/{1}/Stage-Add.pfd", "{0}/Packed/Stage-Add", "StageAddPFD")

	-- TODO: Decompress AR files within PFD

	-- Terrain
	LoadTerrainList("{0}/#{1}/terrain.terrain",
		"{0}/Packed/Stage", "{0}/{1}")
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