CanAddSetLayer = true

function ExtractResources(sourceDir, destDir)
	SetDataType("Forces")
	Extract("{0}/CommonObject.pac", "{0}")
	-- TODO: Finish this
end

function Load(dataDir, cacheDir, stageID)
	SetDataType("Forces")

	-- actstgmission.lua
	local actPth = IOPathCombine(dataDir, "actstgmission.lua")
	local stageDir = "{0}/{1}/"
	local dirName = stageID

	if IOFileExists(actPth) then
		dofile(actPth)

		if mission_all ~= nil and mission_all[1] ~= nil and mission_all[1].missions ~= nil then
			for i, m in ipairs(mission_all[1].missions) do
				if m.name == stageID then
					if m.dir ~= nil then
						dirName = m.dir
						stageDir = "{0}/" .. m.dir .. "/"
					end

					break
				end
			end
		end
	end

	-- Add Resource Directories
	AddResourceDirectory(stageDir .. dirName .. "_obj")
	AddResourceDirectory(stageDir .. dirName .. "_trr_cmn")

	for i = 0, 99 do
		AddResourceDirectory(string.format(
			stageDir .. dirName .. "_trr_s%02d", i))
	end

	-- Miscellaneous (E.G. w5a01/w5a01_misc.pac)
	UIChangeStatus("Extracting Miscellaneous PAC...")
	Extract(stageDir .. dirName .. "_misc.pac",
		stageDir .. dirName .. "_misc", "Miscellaneous")

	-- Object (E.G. w5a01/w5a01_obj.pac)
	UIChangeStatus("Extracting Object PAC...")
	Extract(stageDir .. dirName .. "_obj.pac",
		stageDir .. dirName .. "_obj", "Object")

	-- Terrain Common (E.G. w5a01/w5a01_trr_cmn.pac)
	UIChangeStatus("Extracting Terrain Common PAC...")
	Extract(stageDir .. dirName .. "_trr_cmn.pac",
		stageDir .. dirName .. "_trr_cmn", "TerrainCommon")

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

			local layer = LoadSetLayer(files[i], true)

			-- Change Default Set Layer
			if layer.Name == stageID .. "_obj_area01" then
				ChangeCurrentSetLayer(layer)
			end
		end

		UIHideProgress()
	end

	UIToggleSetsSaving(true)

	-- Sky (E.G. w5a01/w5a01_sky.pac)
	UIChangeStatus("Extracting Sky PAC...")
	Extract(stageDir .. dirName .. "_sky.pac",
		stageDir .. dirName .. "_sky", "Sky")

	-- Terrain Blocks (E.G. w5a01/w5a01_trr_s00.pac)
	UIShowProgress()
	for i = 0, 99 do
		UIChangeProgress((i / 99) * 100)
		UIChangeLoadStatus(string.format("Terrain Sector %02d/99", i))

		local pth = string.format(stageDir .. dirName .. "_trr_s%02d", i)
		local group = "Sector #" .. i
		Extract(pth .. ".pac", pth, "TerrainSector".. i)

		if IODirExists(pth) then
			local files = IOGetFilesInDir(pth, "*.terrain-instanceinfo", false)
			if files ~= nil and #files > 0 then
				for i2 = 1, #files do
					local file = files[i2]

					-- Skip blocks containing "_noGI"
					-- TODO: Load these properly instead
					if not file:find("_noGI") then
						LoadTerrainInstance(file, stageDir .. dirName .. "_trr_cmn", pth, group)
					end
				end
			end
		end
	end
end

function SaveSets(dataDir, cacheDir, stageID)
	-- Set Data (E.G. gedit/w5a01_gedit.pac)
	SetDataType("Forces")
	IODeleteFilesInDir("{0}/gedit/{1}_gedit", ".gedit")
	SaveSetLayers("{0}/gedit/{1}_gedit", "", ".gedit")
	
	-- TODO: Repack
end

function SaveAll(dataDir, cacheDir, stageID)
	-- Materials
	SetDataType("Forces")
	--SaveMaterials("{0}/{1}/", "", ".material")
	-- TODO: Allow editing materials in obj pacs

	-- TODO: Save Models/Instances
	-- TODO: Repack
end

function InitSetObject(obj)
	AddCustomData(obj, "ParentID", "ushort", 0)
	AddCustomData(obj, "ParentGroupID", "ushort", 0)
	AddCustomData(obj, "GroupID", "ushort", 0)
	AddCustomData(obj, "ChildPosOffset", "vector3", "0,0,0")
	AddCustomData(obj, "ChildRotOffset", "vector3", "0,0,0")
	AddCustomData(obj, "RangeIn", "float", 1000)
	AddCustomData(obj, "RangeOut", "float", 1200)

	obj.CustomData["Name"] = GenSetObjectParam("string",
		obj.ObjectType .. tostring(obj.ObjectID))
end