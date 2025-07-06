#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include "atlas_bricks_ball.h"

/**
 * @file components.h
 * @brief Defines ECS components and related types for the game.
 */

namespace game {

    /**
     * @struct brick_coords
     * @brief Stores brick positions and state index.
     */
    using brick_coords = struct {SDL_FRect pos[NUM_BRICK_STATE]{}; int idx = 0; };

    /**
     * @struct Transform
     * @brief Position and angle of an entity.
     */
    using Transform    = struct { SDL_FPoint p; float angle; };
    /**
     * @struct Drawable
     * @brief Sprite part and size for rendering.
     */
    using Drawable     = struct {SDL_FRect part; SDL_FPoint size; };
    /**
     * @struct ChangePart
     * @brief Brick animation state.
     */
    using ChangePart   = struct {brick_coords coords;};
    /**
     * @struct Intent
     * @brief Player or AI movement intent.
     */
    using Intent       = struct { bool up, down, tilt_down, tilt_up; };
    /**
     * @struct Keys
     * @brief Key bindings for player controls.
     */
    using Keys         = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    /**
     * @struct Collider
     * @brief Box2D body for collision.
     */
    using Collider     = struct { b2BodyId body; };
    /**
     * @struct Scorer
     * @brief Box2D shape for scoring.
     */
    using Scorer       = struct { b2ShapeId s; };
    /**
     * @struct Goal
     * @brief Goal state for left/right.
     */
    using Goal         = struct {bool left, right;};
    /**
     * @struct Falling
     * @brief Power-up falling state.
     */
    using Falling      = struct { float vy; bool playerCollectSide;};
    /**
     * @struct PUtimer
     * @brief Power-up timer (hits left).
     */
    using PUtimer      = struct { float hitsLeft; };
    /**
     * @struct TTL
     * @brief Time-to-live (frames left).
     */
    using TTL          = struct { int framesLeft; };
    /**
     * @struct AI
     * @brief AI state for paddle control.
     */
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
