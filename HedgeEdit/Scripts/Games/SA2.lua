function ExtractResources(sourceDir, destDir)
	SetDataType("SA2")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("SA2")

	-- Set Data (E.G. gd_PC/set0003_s.bin)
	local files = IOGetFilesInDir(dataDir,
		"set" .. stageID .. "*.bin", false)

	print("set" .. stageID .. "*.bin")
	print(#files)
	
	if files ~= nil and #files > 0 then
		UIShowProgress()

		for i = 1, #files do
			print(files[i]) -- TODO
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
	-- Set Data (E.G. gd_PC/set0003_s.bin)
	SetDataType("SA2")

	-- TODO: Save Sets
	
	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end