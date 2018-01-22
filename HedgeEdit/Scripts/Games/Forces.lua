function LoadTexturesInDir(dir, loadingTxt)
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

function ExtractResources(sourceDir, destDir)
	SetDataType("Forces")
	Extract("{0}/CommonObject.pac", "{0}")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Forces")

	-- Miscellaneous (E.G. w5a01/w5a01_misc.pac)
	UIChangeStatus("Extracting Miscellaneous PAC...")
	Extract("{0}/{1}/{1}_misc.pac", "{0}/{1}/{1}_misc", "Miscellaneous")

	-- Object (E.G. w5a01/w5a01_obj.pac)
	UIChangeStatus("Extracting Object PAC...")
	Extract("{0}/{1}/{1}_obj.pac", "{0}/{1}/{1}_obj", "Object")

	-- Terrain Common (E.G. w5a01/w5a01_trr_cmn.pac)
	UIChangeStatus("Extracting Terrain Common PAC...")
	Extract("{0}/{1}/{1}_trr_cmn.pac", "{0}/{1}/{1}_trr_cmn", "TerrainCommon")

	-- Set Data (E.G. gedit/w5a01_gedit.pac)
	UIChangeStatus("Extracting Set Data...")
	Extract("{0}/gedit/{1}_gedit.pac", "{0}/gedit/{1}_gedit", "Sets")

	local files = IOGetFilesInDir("{0}/gedit/{1}_gedit",
		"*.gedit", false)
	
	if files ~= nil and #files > 0 then
		UIShowProgress()

		for i = 1, #files do
			UIChangeProgress(((i - 1) / #files) * 100)
			UIChangeLoadStatus(string.format(
				"Set Data %02d/%02d", i, #files))

			LoadSetLayer(files[i], true,
				{ "{0}/{1}/{1}_obj", "{0}/{1}/{1}_trr_cmn" })
		end

		UIHideProgress()
	end

	-- Additional Object Textures
	LoadTexturesInDir("{0}/{1}/{1}_obj", "Object Textures")
	LoadTexturesInDir("{0}/{1}/{1}_trr_cmn", "Terrain Common Textures")

	-- Sky (E.G. w5a01/w5a01_sky.pac)
	UIChangeStatus("Extracting Sky PAC...")
	Extract("{0}/{1}/{1}_sky.pac", "{0}/{1}/{1}_sky", "Sky")

	-- Terrain Blocks (E.G. w5a01/w5a01_trr_s00.pac)
	UIShowProgress()
	for i = 0, 99 do
		UIChangeProgress((i / 99) * 100)
		UIChangeLoadStatus(string.format("Terrain Sector %02d/99", i))

		local pth = string.format("{0}/{1}/{1}_trr_s%02d", i)
		Extract(pth .. ".pac", pth, "TerrainSector".. i)

		if IODirExists(pth) then
			local files = IOGetFilesInDir(pth, "*.terrain-model", false)
			if files ~= nil and #files > 0 then
				for i2 = 1, #files do
					local file = files[i2]

					-- Skip blocks containing "_noGI"
					-- TODO: Load these properly instead
					if not file:find("_noGI") then
						LoadTerrain(file, "{0}/{1}/{1}_trr_cmn")
					end
				end
			end
		end
	end
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data (E.G. gedit/w5a01_gedit.pac)
	SetDataType("Forces")
	SaveSetLayers("{0}/gedit/{1}_gedit", "", ".gedit", true)
	
	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end