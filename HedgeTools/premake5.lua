group("HedgeTools")
    include("HedgeOffsets")
    include("HedgeArcPack")

    -- TODO: Also support non-Windows platforms
    if Target == "windows" then
        include("HedgeEdit")
    end
