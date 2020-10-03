#include "custom/pokemon_extensions.h"
#include "global.h"
#include "pokemon.h"
#include "constants/items.h"

// returns a copy of the created Pokemon
struct Pokemon CreateAndGiveMonToPlayer(u16 species, u8 level)
{
    CreateAndGiveMonToPlayerWithHeldItem(species, level, ITEM_NONE);
}

// returns a copy of the created Pokemon
struct Pokemon CreateAndGiveMonToPlayerWithHeldItem(u16 species, u8 level, u16 itemId)
{
    struct Pokemon mon;
    u8 heldItem[2];
    CreateMon(&mon, species, level, 32, 0, 0, OT_ID_PLAYER_ID, 0);
    heldItem[0] = itemId;
    heldItem[1] = itemId >> 8;

    SetMonData(&mon, MON_DATA_HELD_ITEM, heldItem);

    GiveMonToPlayer(&mon);

    return mon;
}
