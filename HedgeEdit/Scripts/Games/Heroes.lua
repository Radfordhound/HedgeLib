function ExtractResources(sourceDir, destDir)
	SetDataType("Heroes")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Heroes")

	-- Set Data (E.G. s01_P1.bin)
	local files = IOGetFilesInDir(dataDir, stageID .. "_P*.bin", false)
	Log("About to load sets")
	
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
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data (E.G. s01_P1.bin)
	SetDataType("Heroes")
	SaveSetLayers(dataDir, "", ".bin", true)

	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end