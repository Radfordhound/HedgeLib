function ExtractResources(sourceDir, destDir)
	SetDataType("Colors")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Colors")

	-- Set Data (E.G. set/stg110_obj_00.orc)
	UIShowProgress()

	for i = 0, 7 do
		UIChangeProgress((i / 7) * 100)
		UIChangeLoadStatus(string.format(
			"Set Data %02d/%02d", (i + 1), 8))

		local setLayer = LoadSetLayer(string.format(
			"%s/set/%s_obj_%02d.orc", dataDir, stageID, i))
		
		-- Change Layer Name
		if setLayer ~= nil then
			if i == 0 then
				setLayer.Name = "Base"
			elseif i == 1 then
				setLayer.Name = "Custom 1"
			elseif i == 2 then
				setLayer.Name = "Custom 2"
			elseif i == 3 then
				setLayer.Name = "Custom 3"
			elseif i == 4 then
				setLayer.Name = "Gismo"
			elseif i == 5 then
				setLayer.Name = "Design"
			elseif i == 6 then
				setLayer.Name = "Sound"
			elseif i == 7 then
				setLayer.Name = "Navigator"
			end
		end
	end

	UIHideProgress()

	-- TODO
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data (E.G. set/stg110_obj_00.orc)
	SetDataType("Colors")
	--SaveSetLayers(dataDir .. "/set", "", ".orc", true)
	UIShowProgress()

	local layerCount = GetSetLayerCount()
	for i = 1, layerCount do
		UIChangeProgress(((i - 1) / layerCount) * 100)
		UIChangeLoadStatus(string.format(
			"Set Data %02d/%02d", i, layerCount))

		local setLayer = GetSetLayer(i - 1)

		-- Get Layer ID based on name
		if setLayer ~= nil then
			local id = nil

			if setLayer.Name == "Base" then
				id = 0
			elseif setLayer.Name == "Custom 1" then
				id = 1
			elseif setLayer.Name == "Custom 2" then
				id = 2
			elseif setLayer.Name == "Custom 3" then
				id = 3
			elseif setLayer.Name == "Gismo" then
				id = 4
			elseif setLayer.Name == "Design" then
				id = 5
			elseif setLayer.Name == "Sound" then
				id = 6
			elseif setLayer.Name == "Navigator" then
				id = 7
			end

			-- Save Layer
			if id ~= nil then
				SaveSetLayer(string.format("%s/set/%s_obj_%02d.orc",
					dataDir, stageID, id), setLayer)
			else
				LogWarning("WARNING: Skipped Set Layer " .. setLayer.Name ..
					" because its type could not be determined.")
			end
		end
	end

	UIHideProgress()

	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end