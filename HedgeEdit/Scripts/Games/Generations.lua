function Load(dataDir, cacheDir, stageID)
	SetDataType("Gens")

	-- #AR (E.G. #ghz200)
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

			local layer = LoadSetLayer(files[i])
			-- TODO: Remove .set from end of layer name

			-- Change Default Set Layer
			if layer.Name == "setdata_base.set" then
				ChangeCurrentSetLayer(layer)
			end
		end

		UIHideProgress()
	end

	UIToggleSetsSaving(true)

	-- AR (E.G. ghz200)
	UIChangeStatus("Extracting " .. stageID .. "...")
	Extract("{0}/{1}.arl", "{0}/{1}", "AR")

	-- Packed AR (E.G. Packed/ghz200)
	UIChangeStatus("Extracting Packed AR...")
	Extract("{0}/Packed/{1}/{1}.arl", "{0}/Packed/{1}", "PackedAR")
	
	-- Stage PFD (E.G. Packed/Stage)
	UIChangeStatus("Extracting Stage PFD...")
	Extract("{0}/Packed/{1}/Stage.pfd", "{0}/Packed/Stage", "StagePFD")

	-- Stage-Add PFD (E.G. Packed/Stage-Add)
	UIChangeStatus("Extracting Stage-Add PFD...")
	Extract("{0}/Packed/{1}/Stage-Add.pfd", "{0}/Packed/Stage-Add", "StageAddPFD")

	-- TODO: Decompress CAB AR files within PFD

	-- Terrain
	LoadTerrainList("{0}/Packed/{1}/terrain.terrain",
		"{0}/Packed/Stage", "{0}/Packed/{1}")
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data
	SaveSetLayers("{0}/#{1}", "", ".xml", true)
	UIChangeStatus("Repacking #" .. stageID .. "...")
	SetDataType("Gens")
	Repack("{0}/#{1}.ar", "{0}/#{1}", "#AR", false, true)
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end