#ifndef GAMES_H_INCLUDED
#define GAMES_H_INCLUDED
#include "hedgelib/hl_internal.h"

typedef struct HrInstance HrInstance;
typedef struct HrResMgr HrResMgr;

typedef enum GameType
{
/* Auto-generate GameType enum. */
#define GAME(gameID, name) GAME_TYPE_##gameID,
#include "games_autogen.h"

    GAME_TYPE_COUNT
}
GameType;

typedef HlResult(*GameLoaderFunc)(HrInstance* HL_RESTRICT instance,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr);

typedef struct GameInfo
{
    const char* name;
    GameLoaderFunc loader;
}
GameInfo;

/*
    We do GAME_TYPE_COUNT + 1 and append a dummy GameInfo to
    avoid comma issues with auto-generator in game_info.c.
*/
extern const GameInfo GameInfos[GAME_TYPE_COUNT + 1];

extern GameType CurGameType;
extern HlResult StageLoadResult;
extern HlBool IsLoadingStage;

void startLoadingStage(HrInstance* HL_RESTRICT instance, GameType gameType,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr);

void startLoadingStageUTF8(HrInstance* HL_RESTRICT instance, GameType gameType,
    const char* HL_RESTRICT dir, const char* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr);
#endif
