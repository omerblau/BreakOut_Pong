#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include "atlas_bricks_ball.h"

namespace game {

    using brick_coords = struct {SDL_FRect pos[NUM_BRICK_STATE]{}; int idx = 0; };

    /// Components
    using Transform    = struct { SDL_FPoint p; float angle; };
    using Drawable     = struct {SDL_FRect part; SDL_FPoint size; };
    using ChangePart   = struct {brick_coords coords;};
    using Intent       = struct { bool up, down, tilt_down, tilt_up; };
    using Keys         = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    using Collider     = struct { b2BodyId body; };
    using Scorer       = struct { b2ShapeId s; };
    using Goal         = struct {bool left, right;};
    using Falling      = struct { float vy; bool playerCollectSide;};
    using PUtimer      = struct { float hitsLeft; };
    using TTL          = struct { int framesLeft; };
    using AI           = struct {
                            float targetY = -1.0f;
                            int tiltFramesRemaining = 0;
                            int tiltDirection = 0;
                            int cooldownFrames = 1;
                         };
    /// Tags
    using IsCollision  = struct {};
    using Breakable    = struct {};
    using Ball         = struct {};
    using EnlargePU    = struct {};
    using TagLeft = struct {};
    using TagRight = struct {};
    using PU_EnlargeSelf = struct {};
    using PU_ShrinkEnemy = struct {};
    using PU_Coin = struct {};
    using PU_ExtraBall = struct {};
    using leftBallTouchedLast = struct {};
    using rightBallTouchedLast = struct {};
    using ProtectLeft  = struct {};
    using ProtectRight = struct {};
}
