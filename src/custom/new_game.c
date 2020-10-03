#include "custom/new_game.h"
#include "global.h"
#include "bg.h"
#include "field_special_scene.h"
#include "gpu_regs.h"
#include "io_reg.h"
#include "load_save.h"
#include "menu.h"
#include "naming_screen.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "constants/rgb.h"
#include "safari_zone.h"
#include "scanline_effect.h"
#include "constants/songs.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "custom/pokemon_extensions.h"
#include "constants/species.h"
#include "constants/items.h"

//

#include "roamer.h"
#include "pokemon_size_record.h"
#include "script.h"
#include "lottery_corner.h"
#include "play_time.h"
#include "mauville_old_man.h"
#include "match_call.h"
#include "lilycove_lady.h"
#include "pokeblock.h"
#include "dewford_trend.h"
#include "berry.h"
#include "rtc.h"
#include "easy_chat.h"
#include "event_data.h"
#include "money.h"
#include "trainer_hill.h"
#include "tv.h"
#include "coins.h"
#include "mail.h"
#include "battle_records.h"
#include "item.h"
#include "pokedex.h"
#include "apprentice.h"
#include "frontier_util.h"
#include "constants/maps.h"
#include "save.h"
#include "link_rfu.h"
#include "main.h"
#include "contest.h"
#include "item_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_jump.h"
#include "decoration_inventory.h"
#include "secret_base.h"
#include "player_pc.h"
#include "field_specials.h"
#include "berry_powder.h"
#include "mevent.h"
#include "union_room_chat.h"

//

extern const u8 EventScript_ResetAllMapFlags[];

// this file's functions
static void ClearFrontierRecord(void);
static void WarpToPlayersHouse(void);
static void ResetMiniGamesResults(void);

// EWRAM vars
EWRAM_DATA bool8 gDifferentSaveFile = FALSE;
EWRAM_DATA bool8 gEnableContestDebugging = FALSE;

// const rom data
static const struct ContestWinner sContestWinnerPicDummy =
    {
        .monName = _(""),
        .trainerName = _("")};

const u8 BoyOrGirlQuestion[] = _("Are you a boy or a girl?$");
const u8 gText_WhatsYourName[] = _("What's your name?");

static const struct WindowTemplate gTextWindowsTemplate[] = {
    {.bg = 0,
     .tilemapLeft = 2,
     .tilemapTop = 15,
     .width = 27,
     .height = 4,
     .paletteNum = 15,
     .baseBlock = 1},
    {.bg = 0,
     .tilemapLeft = 3,
     .tilemapTop = 5,
     .width = 6,
     .height = 4,
     .paletteNum = 15,
     .baseBlock = 0x6D},
    {.bg = 0,
     .tilemapLeft = 3,
     .tilemapTop = 2,
     .width = 9,
     .height = 10,
     .paletteNum = 15,
     .baseBlock = 0x85},
    DUMMY_WIN_TEMPLATE};

static const u16 bgGradientPal[] = INCBIN_U16("graphics/birch_speech/bg2.gbapal");

static const struct MenuAction sMenuActions_Gender[] = {
    {gText_BirchBoy, NULL},
    {gText_BirchGirl, NULL}};

static const u8 *const gMalePresetNames[] = {
    gText_DefaultNameStu,
    gText_DefaultNameMilton,
    gText_DefaultNameTom,
    gText_DefaultNameKenny,
    gText_DefaultNameReid,
    gText_DefaultNameJude,
    gText_DefaultNameJaxson,
    gText_DefaultNameEaston,
    gText_DefaultNameWalker,
    gText_DefaultNameTeru,
    gText_DefaultNameJohnny,
    gText_DefaultNameBrett,
    gText_DefaultNameSeth,
    gText_DefaultNameTerry,
    gText_DefaultNameCasey,
    gText_DefaultNameDarren,
    gText_DefaultNameLandon,
    gText_DefaultNameCollin,
    gText_DefaultNameStanley,
    gText_DefaultNameQuincy};

static const u8 *const gFemalePresetNames[] = {
    gText_DefaultNameKimmy,
    gText_DefaultNameTiara,
    gText_DefaultNameBella,
    gText_DefaultNameJayla,
    gText_DefaultNameAllie,
    gText_DefaultNameLianna,
    gText_DefaultNameSara,
    gText_DefaultNameMonica,
    gText_DefaultNameCamila,
    gText_DefaultNameAubree,
    gText_DefaultNameRuthie,
    gText_DefaultNameHazel,
    gText_DefaultNameNadine,
    gText_DefaultNameTanja,
    gText_DefaultNameYasmin,
    gText_DefaultNameNicola,
    gText_DefaultNameLillie,
    gText_DefaultNameTerra,
    gText_DefaultNameLucy,
    gText_DefaultNameHalie};

static const struct BgTemplate sMainMenuBgTemplates[] = {
    {.bg = 0,
     .charBaseIndex = 2,
     .mapBaseIndex = 30,
     .screenSize = 0,
     .paletteMode = 0,
     .priority = 0,
     .baseTile = 0},
    {.bg = 1,
     .charBaseIndex = 0,
     .mapBaseIndex = 7,
     .screenSize = 0,
     .paletteMode = 0,
     .priority = 3,
     .baseTile = 0}};

static const struct BgTemplate sNewGameBgTemplate = {
    .bg = 0,
    .charBaseIndex = 3,
    .mapBaseIndex = 30,
    .screenSize = 0,
    .paletteMode = 0,
    .priority = 0,
    .baseTile = 0};

static const u16 sNewGameBgPals[][16] = {
    INCBIN_U16("graphics/birch_speech/bg0.gbapal"),
    INCBIN_U16("graphics/birch_speech/bg1.gbapal")};


static const u32 sNewGameShadowGfx[] = INCBIN_U32("graphics/birch_speech/shadow.4bpp.lz");
static const u32 sBirchSpeechBgMap[] = INCBIN_U32("graphics/birch_speech/map.bin.lz");

static void VBlankCB_MainMenu(void);
static void CB2_MainMenu(void);

// Entry point
static void InitTextWindow(void);
static void LoadMainMenuWindowFrameTiles(u8, u16);
static void ShowDialogueWindow(u8, u8);
static void CreateDialogueWindowBorder(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f);
static void ClearWindow(u8);

// Start Player Fade In
static void Task_NewGame_StartPlayerFadeIn(u8);
static void StartFadeInTarget1OutTarget2(u8, u8);
static void StartFadePlatformOut(u8, u8);
static void Task_WaitForPlayerFadeIn(u8);

// "Are you a boy or a girl?"
static void Task_BoyOrGirl(u8);
static void Task_WaitToShowGenderMenu(u8);
static void ShowGenderMenu(void);
static void DrawMainMenuWindowBorder(const struct WindowTemplate *template, u16 baseTileNum);
static void Task_ChooseGender(u8);
static void ClearGenderWindow(u8, bool8);
static void StartFadeOutTarget1InTarget2(u8, u8);
static void Task_SlideOutOldGenderSprite(u8);
static void Task_SlideInNewGenderSprite(u8);

// "What's your name?"
static void Task_WhatsYourName(u8);
static void Task_WaitForWhatsYourNameToPrint(u8);
static void Task_WaitPressBeforeNameChoice(u8);
static void Task_StartNamingScreen(u8);
static void SetDefaultPlayerName(u8);
static void CB2_ReturnFromNamingScreen(void);
static void Task_ReturnFromNamingScreenShowTextbox(u8);

// Begin New Game
void BeginNewGame(void);

// code
void SetTrainerId(u32 trainerId, u8 *dst)
{
    dst[0] = trainerId;
    dst[1] = trainerId >> 8;
    dst[2] = trainerId >> 16;
    dst[3] = trainerId >> 24;
}

u32 GetTrainerId(u8 *trainerId)
{
    return (trainerId[3] << 24) | (trainerId[2] << 16) | (trainerId[1] << 8) | (trainerId[0]);
}

void CopyTrainerId(u8 *dst, u8 *src)
{
    s32 i;
    for (i = 0; i < TRAINER_ID_LENGTH; i++)
        dst[i] = src[i];
}

static void InitPlayerTrainerId(void)
{
    u32 trainerId = (Random() << 0x10) | GetGeneratedTrainerIdLower();
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

// L=A isnt set here for some reason.
static void SetDefaultOptions(void)
{
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_MID;
    gSaveBlock2Ptr->optionsWindowFrameType = 0;
    gSaveBlock2Ptr->optionsSound = OPTIONS_SOUND_MONO;
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SHIFT;
    gSaveBlock2Ptr->optionsBattleSceneOff = FALSE;
    gSaveBlock2Ptr->regionMapZoom = FALSE;
}

static void ClearPokedexFlags(void)
{
    gUnusedPokedexU8 = 0;
    memset(&gSaveBlock1Ptr->dexCaught, 0, sizeof(gSaveBlock1Ptr->dexCaught));
    memset(&gSaveBlock1Ptr->dexSeen, 0, sizeof(gSaveBlock1Ptr->dexSeen));
}

void ClearAllContestWinnerPics(void)
{
    s32 i;

    ClearContestWinnerPicsInContestHall();
    for (i = 8; i < 13; i++)
        gSaveBlock1Ptr->contestWinners[i] = sContestWinnerPicDummy;
}

static void ClearFrontierRecord(void)
{
    CpuFill32(0, &gSaveBlock2Ptr->frontier, sizeof(gSaveBlock2Ptr->frontier));

    gSaveBlock2Ptr->frontier.opponentNames[0][0] = EOS;
    gSaveBlock2Ptr->frontier.opponentNames[1][0] = EOS;
}

static void WarpToPlayersHouse(void)
{
    SetWarpDestination(MAP_GROUP(HOME_TOWN), MAP_NUM(HOME_TOWN), 0, 2, 2);
    WarpIntoMap();
}

void Sav2_ClearSetDefault(void)
{
    ClearSav2();
    SetDefaultOptions();
}

void ResetMenuAndMonGlobals(void)
{
    gDifferentSaveFile = 0;
    ResetPokedexScrollPositions();
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetBagScrollPositions();
    ResetPokeblockScrollPositions();
}

void NewGameInitData(void)
{
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        RtcReset();

    gDifferentSaveFile = 1;
    gSaveBlock2Ptr->encryptionKey = 0;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetPokedex();
    ClearFrontierRecord();
    ClearSav1();
    ClearMailData();
    gSaveBlock2Ptr->specialSaveWarpFlags = 0;
    gSaveBlock2Ptr->gcnLinkFlags = 0;
    InitPlayerTrainerId();
    PlayTimeCounter_Reset();
    ClearPokedexFlags();
    InitEventData();
    ClearTVShowData();
    ResetGabbyAndTy();
    ClearSecretBases();
    ClearBerryTrees();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    SetCoins(0);
    ResetLinkContestBoolean();
    ResetGameStats();
    ClearAllContestWinnerPics();
    ClearPlayerLinkBattleRecords();
    InitSeedotSizeRecord();
    InitLotadSizeRecord();
    gPlayerPartyCount = 0;
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    ClearRoamerData();
    ClearRoamerLocationData();
    gSaveBlock1Ptr->registeredItem = 0;
    ClearBag();
    NewGameInitPCItems();
    ClearPokeblocks();
    ClearDecorationInventories();
    InitEasyChatPhrases();
    SetMauvilleOldMan();
    InitDewfordTrend();
    ResetFanClub();
    ResetLotteryCorner();
    WarpToPlayersHouse();
    ScriptContext2_RunNewScript(EventScript_ResetAllMapFlags);
    ResetMiniGamesResults();
    InitUnionRoomChatRegisteredTexts();
    InitLilycoveLady();
    ResetAllApprenticeData();
    ClearRankingHallRecords();
    InitMatchCallCounters();
    sub_801AFD8();
    WipeTrainerNameRecords();
    ResetTrainerHillResults();
    ResetContestLinkResults();
}

static void ResetMiniGamesResults(void)
{
    CpuFill16(0, &gSaveBlock2Ptr->berryCrush, sizeof(struct BerryCrush));
    SetBerryPowder(&gSaveBlock2Ptr->berryCrush.berryPowderAmount, 0);
    ResetPokeJumpResults();
    CpuFill16(0, &gSaveBlock2Ptr->berryPick, sizeof(struct BerryPickingResults));
}

// New Game - Entry Point
//
void Task_NewGame(u8 taskId)
{
    InitTextWindow();
    gTasks[taskId].data[4] = -60; // position platform/spotlight
    SetGpuReg(0x14, -60);
    gTasks[taskId].func = Task_NewGame_StartPlayerFadeIn;
}
//
static void InitTextWindow(void)
{
    InitWindows(gTextWindowsTemplate);
    LoadMainMenuWindowFrameTiles(0, 0xF3);
    LoadMessageBoxGfx(0, 0xFC, 0xF0);
    ShowDialogueWindow(0, 1);
    PutWindowTilemap(0);
    CopyWindowToVram(0, 2);
    ClearWindow(0);
}
//
static void LoadMainMenuWindowFrameTiles(u8 bgId, u16 tileOffset)
{
    LoadBgTiles(bgId, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, tileOffset);
    LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, 32, 32);
}
//
static void ShowDialogueWindow(u8 windowId, u8 copyToVram)
{
    CallWindowFunction(windowId, CreateDialogueWindowBorder);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    PutWindowTilemap(windowId);
    if (copyToVram == TRUE)
        CopyWindowToVram(windowId, 3);
}
//
static void CreateDialogueWindowBorder(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f)
{
    FillBgTilemapBufferRect(a, 0xFD, b - 2, c - 1, 1, 1, f);
    FillBgTilemapBufferRect(a, 0xFF, b - 1, c - 1, 1, 1, f);
    FillBgTilemapBufferRect(a, 0x100, b, c - 1, d, 1, f);
    FillBgTilemapBufferRect(a, 0x101, b + d - 1, c - 1, 1, 1, f);
    FillBgTilemapBufferRect(a, 0x102, b + d, c - 1, 1, 1, f);
    FillBgTilemapBufferRect(a, 0x103, b - 2, c, 1, 5, f);
    FillBgTilemapBufferRect(a, 0x105, b - 1, c, d + 1, 5, f);
    FillBgTilemapBufferRect(a, 0x106, b + d, c, 1, 5, f);
    FillBgTilemapBufferRect(a, BG_TILE_V_FLIP(0xFD), b - 2, c + e, 1, 1, f);
    FillBgTilemapBufferRect(a, BG_TILE_V_FLIP(0xFF), b - 1, c + e, 1, 1, f);
    FillBgTilemapBufferRect(a, BG_TILE_V_FLIP(0x100), b, c + e, d - 1, 1, f);
    FillBgTilemapBufferRect(a, BG_TILE_V_FLIP(0x101), b + d - 1, c + e, 1, 1, f);
    FillBgTilemapBufferRect(a, BG_TILE_V_FLIP(0x102), b + d, c + e, 1, 1, f);
}
//
static void ClearWindow(u8 windowId)
{
    u8 bgColor = GetFontAttribute(1, FONTATTR_COLOR_BACKGROUND);
    u8 maxCharWidth = GetFontAttribute(1, FONTATTR_MAX_LETTER_WIDTH);
    u8 maxCharHeight = GetFontAttribute(1, FONTATTR_MAX_LETTER_HEIGHT);
    u8 winWidth = GetWindowAttribute(windowId, WINDOW_WIDTH);
    u8 winHeight = GetWindowAttribute(windowId, WINDOW_HEIGHT);

    FillWindowPixelRect(windowId, bgColor, 0, 0, maxCharWidth * winWidth, maxCharHeight * winHeight);
    CopyWindowToVram(windowId, 2);
}
//
// | |
// V V
//
#define tPlayerSpriteId data[2]
#define tBG1HOFS data[4]
#define tIsDoneFadingSprites data[5]
#define tPlayerGender data[6]
#define tTimer data[7]
#define tBirchSpriteId data[8]
#define tLotadSpriteId data[9]
#define tBrendanSpriteId data[10]
#define tMaySpriteId data[11]
//
// | |
// V V
//
// Start Player Fade In
//
static void Task_NewGame_StartPlayerFadeIn(u8 taskId)
{
    u8 spriteId = gTasks[taskId].tBrendanSpriteId;

    gSprites[spriteId].pos1.x = 180;
    gSprites[spriteId].pos1.y = 60;
    gSprites[spriteId].invisible = FALSE;
    gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
    gTasks[taskId].tPlayerSpriteId = spriteId;
    gTasks[taskId].tPlayerGender = MALE;
    StartFadeInTarget1OutTarget2(taskId, 1);
    StartFadePlatformOut(taskId, 1);
    gTasks[taskId].func = Task_WaitForPlayerFadeIn;
}
//
static void Task_WaitForPlayerFadeIn(u8 taskId)
{
    if (gTasks[taskId].tIsDoneFadingSprites)
    {
        gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
        gTasks[taskId].func = Task_BoyOrGirl;
    }
}
//
// "Are you a boy or a girl?"
//
static void Task_BoyOrGirl(u8 taskId)
{
    ClearWindow(0);
    StringExpandPlaceholders(gStringVar4, BoyOrGirlQuestion);
    AddTextPrinterForMessage(1);
    gTasks[taskId].func = Task_WaitToShowGenderMenu;
}
//
static void Task_WaitToShowGenderMenu(u8 taskId)
{
    if (!RunTextPrintersAndIsPrinter0Active())
    {
        ShowGenderMenu();
        gTasks[taskId].func = Task_ChooseGender;
    }
}
//
static void ShowGenderMenu(void) {
    DrawMainMenuWindowBorder(&gTextWindowsTemplate[1], 0xF3);
    FillWindowPixelBuffer(1, PIXEL_FILL(1));
    PrintMenuTable(1, 2, sMenuActions_Gender);
    InitMenuInUpperLeftCornerPlaySoundWhenAPressed(1, 2, 0);
    PutWindowTilemap(1);
    CopyWindowToVram(1, 3);
}
//
static void Task_ChooseGender(u8 taskId)
{
    int gender = Menu_ProcessInputNoWrap(); // Process Boy/Girl choice
    int gender2;

    switch (gender)
    {
    case MALE:
        PlaySE(SE_SELECT);
        gSaveBlock2Ptr->playerGender = gender;
        ClearGenderWindow(1, 1);
        gTasks[taskId].func = Task_WhatsYourName;
        break;
    case FEMALE:
        PlaySE(SE_SELECT);
        gSaveBlock2Ptr->playerGender = gender;
        ClearGenderWindow(1, 1);
        gTasks[taskId].func = Task_WhatsYourName;
        break;
    }
    gender2 = Menu_GetCursorPos();
    if (gender2 != gTasks[taskId].tPlayerGender)
    {
        gTasks[taskId].tPlayerGender = gender2;
        gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
        StartFadeOutTarget1InTarget2(taskId, 0);
        gTasks[taskId].func = Task_SlideOutOldGenderSprite;
    }
}
//
static void Task_SlideOutOldGenderSprite(u8 taskId)
{
    u8 spriteId = gTasks[taskId].tPlayerSpriteId;
    if (gTasks[taskId].tIsDoneFadingSprites == 0)
    {
        gSprites[spriteId].pos1.x += 4;
    }
    else
    {
        gSprites[spriteId].invisible = TRUE;
        if (gTasks[taskId].tPlayerGender != MALE)
            spriteId = gTasks[taskId].tMaySpriteId;
        else
            spriteId = gTasks[taskId].tBrendanSpriteId;
        gSprites[spriteId].pos1.x = 240;
        gSprites[spriteId].pos1.y = 60;
        gSprites[spriteId].invisible = FALSE;
        gTasks[taskId].tPlayerSpriteId = spriteId;
        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
        StartFadeInTarget1OutTarget2(taskId, 0);
        gTasks[taskId].func = Task_SlideInNewGenderSprite;
    }
}
//
static void Task_SlideInNewGenderSprite(u8 taskId)
{
    u8 spriteId = gTasks[taskId].tPlayerSpriteId;

    if (gSprites[spriteId].pos1.x > 180)
    {
        gSprites[spriteId].pos1.x -= 4;
    }
    else
    {
        gSprites[spriteId].pos1.x = 180;
        if (gTasks[taskId].tIsDoneFadingSprites)
        {
            gSprites[spriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
            gTasks[taskId].func = Task_ChooseGender;
        }
    }
}
//
static void Task_WhatsYourName(u8 taskId)
{
    ClearWindow(0);
    StringExpandPlaceholders(gStringVar4, gText_WhatsYourName);
    AddTextPrinterForMessage(1);
    gTasks[taskId].func = Task_WaitForWhatsYourNameToPrint;
}
//
static void Task_WaitForWhatsYourNameToPrint(u8 taskId)
{
    if (!RunTextPrintersAndIsPrinter0Active())
    {
        gTasks[taskId].func = Task_WaitPressBeforeNameChoice;
    }
}
//
static void Task_WaitPressBeforeNameChoice(u8 taskId)
{
    if ((JOY_NEW(A_BUTTON)) || (JOY_NEW(B_BUTTON)))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_StartNamingScreen;
    }
}
//
static void Task_StartNamingScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAllWindowBuffers();
        SetDefaultPlayerName(Random() % 20);
        DestroyTask(taskId);
        DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName, gSaveBlock2Ptr->playerGender, 0, 0, CB2_ReturnFromNamingScreen);
    }
}
//
static void SetDefaultPlayerName(u8 nameId)
{
    const u8 *name;
    u8 i;

    if (gSaveBlock2Ptr->playerGender == MALE)
        name = gMalePresetNames[nameId];
    else
        name = gFemalePresetNames[nameId];
    for (i = 0; i < 7; i++)
        gSaveBlock2Ptr->playerName[i] = name[i];
    gSaveBlock2Ptr->playerName[7] = 0xFF;
}
//
static void CB2_ReturnFromNamingScreen(void)
{
    u16 savedIme;

    SetVBlankCallback(NULL);
    savedIme = REG_IME;
    REG_IME = 0;
    REG_IE |= 1;
    REG_IME = savedIme;

    BeginNewGame();
}
//
void BeginNewGame(void)
{
    FieldClearVBlankHBlankCallbacks();
    StopMapMusic();
    ResetSafariZoneFlag();
    NewGameInitData();
    ResetInitialPlayerAvatarState();
    PlayTimeCounter_Start();
    ScriptContext1_Init();
    ScriptContext2_Disable();
    gFieldCallback = NULL; //ExecuteTruckSequence;
    gFieldCallback2 = NULL;
    DoMapLoadLoop(&gMain.state);
    SetFieldVBlankCallback();
    SetMainCallback1(CB1_Overworld);
    SetMainCallback2(CB2_Overworld);

    CreateAndGiveMonToPlayer(SPECIES_TOTODILE, 4); // DEBUG
    AddBagItem(ITEM_POTION, 6);
}

#undef tPlayerSpriteId
#undef tBG1HOFS
#undef tPlayerGender
#undef tBirchSpriteId
#undef tLotadSpriteId
#undef tBrendanSpriteId
#undef tMaySpriteId
//
#define tMainTask data[0]
#define tAlphaCoeff1 data[1]
#define tAlphaCoeff2 data[2]
#define tDelay data[3]
#define tDelayTimer data[4]

static void Task_FadeInTarget1OutTarget2(u8 taskId)
{
    int alphaCoeff2;

    if (gTasks[taskId].tAlphaCoeff1 == 16)
    {
        gTasks[gTasks[taskId].tMainTask].tIsDoneFadingSprites = TRUE;
        DestroyTask(taskId);
    }
    else if (gTasks[taskId].tDelayTimer)
    {
        gTasks[taskId].tDelayTimer--;
    }
    else
    {
        gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
        gTasks[taskId].tAlphaCoeff1++;
        gTasks[taskId].tAlphaCoeff2--;
        alphaCoeff2 = gTasks[taskId].tAlphaCoeff2 << 8;
        SetGpuReg(REG_OFFSET_BLDALPHA, gTasks[taskId].tAlphaCoeff1 + alphaCoeff2);
    }
}
//
static void StartFadeInTarget1OutTarget2(u8 taskId, u8 delay)
{
    u8 taskId2;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT1_OBJ);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
    SetGpuReg(REG_OFFSET_BLDY, 0);
    gTasks[taskId].tIsDoneFadingSprites = 0;
    taskId2 = CreateTask(Task_FadeInTarget1OutTarget2, 0);
    gTasks[taskId2].tMainTask = taskId;
    gTasks[taskId2].tAlphaCoeff1 = 0;
    gTasks[taskId2].tAlphaCoeff2 = 16;
    gTasks[taskId2].tDelay = delay;
    gTasks[taskId2].tDelayTimer = delay;
}


static void Task_FadeOutTarget1InTarget2(u8 taskId)
{
    int alphaCoeff2;

    if (gTasks[taskId].tAlphaCoeff1 == 0)
    {
        gTasks[gTasks[taskId].tMainTask].tIsDoneFadingSprites = TRUE;
        DestroyTask(taskId);
    }
    else if (gTasks[taskId].tDelayTimer)
    {
        gTasks[taskId].tDelayTimer--;
    }
    else
    {
        gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
        gTasks[taskId].tAlphaCoeff1--;
        gTasks[taskId].tAlphaCoeff2++;
        alphaCoeff2 = gTasks[taskId].tAlphaCoeff2 << 8;
        SetGpuReg(REG_OFFSET_BLDALPHA, gTasks[taskId].tAlphaCoeff1 + alphaCoeff2);
    }
}
//
static void StartFadeOutTarget1InTarget2(u8 taskId, u8 delay)
{
    u8 taskId2;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT1_OBJ);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
    SetGpuReg(REG_OFFSET_BLDY, 0);
    gTasks[taskId].tIsDoneFadingSprites = 0;
    taskId2 = CreateTask(Task_FadeOutTarget1InTarget2, 0);
    gTasks[taskId2].tMainTask = taskId;
    gTasks[taskId2].tAlphaCoeff1 = 16;
    gTasks[taskId2].tAlphaCoeff2 = 0;
    gTasks[taskId2].tDelay = delay;
    gTasks[taskId2].tDelayTimer = delay;
}

#undef tMainTask
#undef tAlphaCoeff1
#undef tAlphaCoeff2
#undef tDelay
#undef tDelayTimer
//
#undef tIsDoneFadingSprites
//
#define tMainTask data[0]
#define tPalIndex data[1]
#define tDelayBefore data[2]
#define tDelay data[3]
#define tDelayTimer data[4]

// NewGame_StartPlayerFadeIn
//
static void Task_FadePlatformOut(u8 taskId)
{
    if (gTasks[taskId].tPalIndex == 0)
    {
        DestroyTask(taskId);
    }
    else
    {
        gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
        gTasks[taskId].tPalIndex--;
        LoadPalette(&bgGradientPal[gTasks[taskId].tPalIndex], 1, 16);
    }
}
//
static void StartFadePlatformOut(u8 taskId, u8 delay)
{
    u8 taskId2;

    taskId2 = CreateTask(Task_FadePlatformOut, 0);
    gTasks[taskId2].tMainTask = taskId;
    gTasks[taskId2].tPalIndex = 8;
    gTasks[taskId2].tDelayBefore = 8;
    gTasks[taskId2].tDelay = delay;
    gTasks[taskId2].tDelayTimer = delay;
}


#undef tMainTask
#undef tPalIndex
#undef tDelayBefore
#undef tDelay
#undef tDelayTimer


#undef tTimer

static void DrawMainMenuWindowBorder(const struct WindowTemplate *template, u16 baseTileNum)
{
    u16 r9 = 1 + baseTileNum;
    u16 r10 = 2 + baseTileNum;
    u16 sp18 = 3 + baseTileNum;
    u16 spC = 5 + baseTileNum;
    u16 sp10 = 6 + baseTileNum;
    u16 sp14 = 7 + baseTileNum;
    u16 r6 = 8 + baseTileNum;

    FillBgTilemapBufferRect(template->bg, baseTileNum, template->tilemapLeft - 1, template->tilemapTop - 1, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, r9, template->tilemapLeft, template->tilemapTop - 1, template->width, 1, 2);
    FillBgTilemapBufferRect(template->bg, r10, template->tilemapLeft + template->width, template->tilemapTop - 1, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, sp18, template->tilemapLeft - 1, template->tilemapTop, 1, template->height, 2);
    FillBgTilemapBufferRect(template->bg, spC, template->tilemapLeft + template->width, template->tilemapTop, 1, template->height, 2);
    FillBgTilemapBufferRect(template->bg, sp10, template->tilemapLeft - 1, template->tilemapTop + template->height, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, sp14, template->tilemapLeft, template->tilemapTop + template->height, template->width, 1, 2);
    FillBgTilemapBufferRect(template->bg, r6, template->tilemapLeft + template->width, template->tilemapTop + template->height, 1, 1, 2);
    CopyBgTilemapBufferToVram(template->bg);
}

// "Are you a boy or a girl?"
static void ClearGenderWindowTilemap(u8 a, u8 b, u8 c, u8 d, u8 e, u8 unused)
{
    FillBgTilemapBufferRect(a, 0, b + 0xFF, c + 0xFF, d + 2, e + 2, 2);
}
//
static void ClearGenderWindow(u8 windowId, bool8 copyToVram)
{
    CallWindowFunction(windowId, ClearGenderWindowTilemap);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    ClearWindowTilemap(windowId);
    if (copyToVram == TRUE)
        CopyWindowToVram(windowId, 3);
}

static void VBlankCB_MainMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_MainMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}
