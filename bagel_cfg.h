//bagel_cfg.h file
// @formatter:off
#pragma once

#include "Element.h"

constexpr Bagel Params {
    .DynamicResize      = true,
    .IdBagSize          = 16,
    .InitialEntities    = 64,
    .InitialPackedSize  = 32,
    .MaxComponents      = 32
};

// — sparse storage
BAGEL_STORAGE(element::UIIntent,     SparseStorage)
BAGEL_STORAGE(element::MouseInput,   SparseStorage)
BAGEL_STORAGE(element::CurrentLevel, SparseStorage)
BAGEL_STORAGE(element::SpawnState,   SparseStorage)

// — packed storage
BAGEL_STORAGE(element::Transform,     PackedStorage)
BAGEL_STORAGE(element::Drawable,      PackedStorage)
BAGEL_STORAGE(element::Velocity,      PackedStorage)
BAGEL_STORAGE(element::WaypointIndex, PackedStorage)
BAGEL_STORAGE(element::HP,            PackedStorage)
BAGEL_STORAGE(element::Gold,          PackedStorage)
BAGEL_STORAGE(element::Gold_Bounty,   PackedStorage)
BAGEL_STORAGE(element::Speed,         PackedStorage)
BAGEL_STORAGE(element::Range,         PackedStorage)
BAGEL_STORAGE(element::Damage,        PackedStorage)
BAGEL_STORAGE(element::FireRate,      PackedStorage)
BAGEL_STORAGE(element::Target,        PackedStorage)
BAGEL_STORAGE(element::TravelTime,    PackedStorage)

// — tagged storage
BAGEL_STORAGE(element::Creep_Tag,        TaggedStorage)
BAGEL_STORAGE(element::Player_Tag,       TaggedStorage)
BAGEL_STORAGE(element::Mouse_Tag,        TaggedStorage)
BAGEL_STORAGE(element::UIButton_Tag,     TaggedStorage)
BAGEL_STORAGE(element::Arrow_Tag,        TaggedStorage)
BAGEL_STORAGE(element::Cannon_Tag,       TaggedStorage)
BAGEL_STORAGE(element::Air_Tag,          TaggedStorage)
BAGEL_STORAGE(element::NextLevel_Tag,    TaggedStorage)
BAGEL_STORAGE(element::GameState_Tag,    TaggedStorage)
BAGEL_STORAGE(element::SpawnManager_Tag, TaggedStorage)
BAGEL_STORAGE(element::Bullet_Tag,       TaggedStorage)
// @formatter:on
