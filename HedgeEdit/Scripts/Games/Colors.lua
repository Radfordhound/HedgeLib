local layerNames = { "Base", "Custom 1", "Custom 2", "Custom 3",
	"Gismo", "Design", "Sound", "Navigator" }

local layerIDs = {}
for i, v in ipairs(layerNames) do
	layerIDs[v] = i - 1
	print("Layer: " .. v .. " = " .. tostring(i - 1))
end

-- Callbacks
function ExtractResources(sourceDir, destDir)
	SetDataType("Colors")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Colors")

	-- Set Data (E.G. set/stg110_obj_00.orc)
	UIShowProgress()

	local currentLayer = nil
	for i = 0, 7 do
		UIChangeProgress((i / 7) * 100)
		UIChangeLoadStatus(string.format(
			"Set Data %02d/%02d", (i + 1), 8))

		local setLayer = LoadSetLayer(string.format(
			"%s/set/%s_obj_%02d.orc", dataDir, stageID, i))
		
		-- Change Layer Name
		if setLayer ~= nil then
			setLayer.Name = layerNames[i + 1]
		else
			setLayer = AddSetLayer(layerNames[i + 1])
		end

		if currentLayer == nil then
			currentLayer = setLayer
		end
	end

	ChangeCurrentSetLayer(currentLayer)

	UIHideProgress()
	UIToggleSetsSaving(true)

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
		if setLayer ~= nil and #setLayer.Objects > 0 then
			-- Save Layer
			if layerIDs[setLayer.Name] ~= nil then
				SaveSetLayer(string.format("%s/set/%s_obj_%02d.orc",
					dataDir, stageID, layerIDs[setLayer.Name]), setLayer)
			else
				LogWarning("WARNING: Skipped Set Layer " .. setLayer.Name ..
					" because its type could not be determined.")
			end
		else
			LogWarning("WARNING: Skipped Set Layer " .. setLayer.Name ..
				" because it was null or contained no objects.")
		end
	end

	UIHideProgress()

	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- TODO
end

function InitSetObject(obj, template)
	AddCustomData(obj, "Unknown1", "ushort", 0)
	AddCustomData(obj, "Unknown2", "uint", 0)
	AddCustomData(obj, "Unknown3", "uint", 0)
	AddCustomData(obj, "Unknown4", "float", 0)
	AddCustomData(obj, "RangeIn", "float", 1000)
	AddCustomData(obj, "RangeOut", "float", 1200)

	-- TODO: In LW version of this function un-comment this line:
	--AddCustomData(obj, "Parent", "uint", 0)
end