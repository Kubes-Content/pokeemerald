#ifndef GUARD_POKEMON_EXTENSIONS_H
#define GUARD_POKEMON_EXTENSIONS_H

#include "gba/types.h"

struct Pokemon CreateAndGiveMonToPlayer(u16 species, u8 level);
struct Pokemon CreateAndGiveMonToPlayerWithHeldItem(u16 species, u8 level, u16 itemId);

#endif //GUARD_POKEMON_EXTENSIONS_H
