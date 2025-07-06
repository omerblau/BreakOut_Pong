/* Auto‑generated from breakout layout – DO NOT EDIT MANUALLY */

#pragma once
#include <SDL3/SDL_rect.h>

static constexpr int NUM_BRICK_STATE = 2;
static inline constexpr SDL_FRect BALL_COORDS{457, 1136, 107, 103};
static inline constexpr SDL_FRect PAD_COORDS{260, 1312, 505, 75};

static inline constexpr SDL_FRect RED_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 70, 135, 80},
    {70,  890, 135, 80}
};

static inline constexpr SDL_FRect GREEN_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 570, 135, 80},
    {70, 1100, 135, 80}
};

static inline constexpr SDL_FRect ORANGE_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 270, 135, 80},
    {70,  1005, 135, 80}
};

static inline constexpr SDL_FRect YELLOW_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 770, 135, 80},
    {70, 1200, 135, 80}
};

static inline constexpr SDL_FRect PROTECT_ACTIVE{255, 70, 135, 80};
static inline constexpr SDL_FRect PROTECT_PASSIVE{255, 170, 135, 80};

static inline constexpr SDL_FRect PAD_SHORT_COORDS{570, 1220, 192, 77};  // short paddle (right of ball)
static inline constexpr SDL_FRect PAD_LONG_COORDS { 48, 1410, 720, 77};  // very long paddle (bottom)

static inline constexpr SDL_FRect POWERUP_SHRINK{797, 1210, 74, 74};  // "> <" symbol, slot 1
static inline constexpr SDL_FRect POWERUP_ENLARGE{877, 1210, 74, 74};  // "< >" symbol, slot 2
static inline constexpr SDL_FRect POWERUP_COIN{797, 1290, 74, 74};  // coin/dollar symbol, slot 3
static inline constexpr SDL_FRect POWERUP_4_COORDS{877, 1290, 74, 74};  // empty slot, rename later
static inline constexpr SDL_FRect POWERUP_5_COORDS{797, 1370, 74, 74};  // empty slot, rename later
static inline constexpr SDL_FRect POWERUP_6_COORDS{877, 1370, 74, 74};  // empty slot, rename later

