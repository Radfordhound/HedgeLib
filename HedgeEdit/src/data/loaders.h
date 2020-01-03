#pragma once
#include "HedgeLib/String.h"

#define LOAD_FUNC Load(GFX::Instance& inst,\
    const hl::nchar* dir, const hl::nchar* name)

#define DECLARE_LOADER(name) struct name { static void LOAD_FUNC; }
#define DEFINE_LOADER(name) void name::LOAD_FUNC

namespace HedgeEdit
{
    namespace GFX
    {
        class Instance;
    }

    namespace Data
    {
        DECLARE_LOADER(LWLoader);
        DECLARE_LOADER(ForcesLoader);

        template<typename Loader>
        void LoadStage(GFX::Instance& inst, const hl::nchar* dir, const hl::nchar* name)
        {
            inst.Clear();
            Loader::Load(inst, dir, name);
        }

#ifdef _WIN32
        template<typename Loader>
        void LoadStage(GFX::Instance& inst, const char* dir, const char* name)
        {
            inst.Clear();

            std::unique_ptr<hl::nchar[]> nativeDir = hl::StringConvertUTF8ToNativePtr(dir);
            std::unique_ptr<hl::nchar[]> nativeName = hl::StringConvertUTF8ToNativePtr(name);

            Loader::Load(inst, nativeDir.get(), nativeName.get());
        }
#endif
    }
}
