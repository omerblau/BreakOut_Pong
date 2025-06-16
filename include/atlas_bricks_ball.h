/* Auto‑generated from breakout layout – DO NOT EDIT MANUALLY */
#pragma once
#include <SDL3/SDL_rect.h>
static constexpr int NUM_BRICK_STATE = 2;
inline constexpr SDL_FRect BALL_COORDS{457, 1136, 107, 103};
inline constexpr SDL_FRect PAD_COORDS{260, 1312, 505, 75};

inline constexpr SDL_FRect RED_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 70, 135, 80},
    {70,  890, 135, 80}
};
inline constexpr SDL_FRect GREEN_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 570, 135, 80},
    {70, 1100, 135, 80}
};
inline constexpr SDL_FRect ORANGE_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 270, 135, 80},
    {70,  1005, 135, 80}
};
inline constexpr SDL_FRect YELLOW_BRICK_COORDS[NUM_BRICK_STATE] = {
    {70, 770, 135, 80},
    {70, 1200, 135, 80}
};