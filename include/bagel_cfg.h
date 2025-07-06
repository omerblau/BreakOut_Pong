#pragma once
#include "components.h"

using namespace game;

constexpr Bagel Params {
    .DynamicResize      = true,
    .IdBagSize          = 120,
    .InitialEntities    = 100,
    .InitialPackedSize  = 32,
    .MaxComponents      = 32
};

BAGEL_STORAGE(IsCollision, TaggedStorage);
BAGEL_STORAGE(Breakable, TaggedStorage);
BAGEL_STORAGE(Ball, TaggedStorage);
BAGEL_STORAGE(EnlargePU, TaggedStorage);
BAGEL_STORAGE(TagLeft, TaggedStorage);
BAGEL_STORAGE(TagRight, TaggedStorage);
BAGEL_STORAGE(PU_EnlargeSelf, TaggedStorage);
BAGEL_STORAGE(PU_ShrinkEnemy, TaggedStorage);
BAGEL_STORAGE(PU_Coin, TaggedStorage);
BAGEL_STORAGE(PU_ExtraBall, TaggedStorage);
BAGEL_STORAGE(leftBallTouchedLast, TaggedStorage);
BAGEL_STORAGE(rightBallTouchedLast, TaggedStorage);
BAGEL_STORAGE(ProtectLeft, TaggedStorage);
BAGEL_STORAGE(ProtectRight, TaggedStorage);