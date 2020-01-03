#include "loaders.h"
#include "PACx.h"
#include "../gfx/instance.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/Path.h"

namespace HedgeEdit::Data
{
    DEFINE_LOADER(LWLoader)
    {
        // Get base path (dataDir/stageDir/name)
        // TODO: Change these to use normal strings later since you're re-allocating a bunch anyway
        std::unique_ptr<hl::nchar[]> basePath = hl::PathCombinePtr(dir, name);
        basePath = hl::PathCombinePtr(basePath.get(), name);

        // Load terrain common pacs
        std::unique_ptr<hl::nchar[]> filePath = hl::StringJoinPtr(
            basePath.get(), HL_NTEXT("_trr_cmn.pac"));

        LoadPACxV2Archive(inst, filePath.get());

        // Load far terrain pacs
        filePath = hl::StringJoinPtr(basePath.get(), HL_NTEXT("_far.pac"));
        LoadPACxV2Archive(inst, filePath.get());

        // Load object pacs
        filePath = hl::StringJoinPtr(basePath.get(), HL_NTEXT("_obj.pac"));
        LoadPACxV2Archive(inst, filePath.get());

        // TODO: Load misc pacs
        // TODO: Load sky pacs
    }

    DEFINE_LOADER(ForcesLoader)
    {
        // w5a01_far.pac (TODO: CHECK TO SEE IF FORCES STILL ATTEMPTS TO LOAD THIS)

        // w5a01_misc.pac
        // w5a01_obj.pac
        // w5a01_sky.pac
        // w5a01_trr_cmn.pac
        // w5a01_trr_sXX.pac (DEFINED BY LUA)

        // Get base path (dataDir/stageDir/name)
        // TODO: Change these to use normal strings later since you're re-allocating a bunch anyway
        std::unique_ptr<hl::nchar[]> basePath = hl::PathCombinePtr(dir, name);
        basePath = hl::PathCombinePtr(basePath.get(), name);

        // Load terrain common pacs
        std::unique_ptr<hl::nchar[]> filePath = hl::StringJoinPtr(
            basePath.get(), HL_NTEXT("_trr_cmn.pac"));

        LoadPACxV3Archive(inst, filePath.get());

        // Load terrain sectors
        // TODO: Load actstgmission.lua and use sector information to load
        // pacs; fallback to loading all sectors if lua can't be found.

        /*filePath = hl::StringJoinPtr(basePath.get(), HL_NTEXT("_trr_s01.pac"));
        LoadPACxV3Archive(inst, filePath.get());*/

        //// Load far terrain pacs
        //filePath = hl::StringJoinPtr(basePath.get(), HL_NTEXT("_far.pac"));
        //LoadPACxV2Archive(inst, filePath.get());

        // Load object pacs
        filePath = hl::StringJoinPtr(basePath.get(), HL_NTEXT("_obj.pac"));
        LoadPACxV3Archive(inst, filePath.get());

        // TODO: Load misc pacs
    }
}
