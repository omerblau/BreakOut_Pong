// This is the newgame.h file

#pragma once
#include "bagel.h"
#include "../res/atlas_bricks_ball.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

    // @formatter:off
using namespace bagel;

namespace game {

    using Transform = struct { SDL_FPoint p; float a; };
    using Drawable = struct { SDL_FRect part; SDL_FPoint size; };
    using Intent = struct { bool up, down, tilt_down, tilt_up; };
    using Keys = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    using Collider = struct { b2BodyId b; };
    using Scorer = struct { b2ShapeId s; };

    class Game {
    public:
        Game();
        ~Game();
        bool valid() const;
        void run() const;

    private:
        /// systems
        void box_system() const;
        void input_system() const;
        void move_system() const;
        void draw_system() const;

        /// factories
        void createBall() const;
        void createBrick(const SDL_FPoint &pos) const;
        void createPad(const SDL_FRect&, const SDL_FPoint&, const Keys&) const;

        /// init game
        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;
        void createPads() const;
        void placeBricks() const;

        static constexpr int WIN_WIDTH = 1500;
        static constexpr int WIN_HEIGHT = 1000;
        static constexpr int FPS = 60;

        static constexpr float GAME_FRAME = 1000.f / FPS;
        static constexpr float RAD_TO_DEG = 57.2958f;

        static constexpr int   PAD_Y_MARGIN    = 200;
        static constexpr float BALL_INIT_MPS   = 3.0f;    // 3 m/s ≈ 30 px/s

        static constexpr float BOX_SCALE        = 10.0f;   // 1 m = 10 px
        static constexpr float BALL_TEX_SCALE   = 0.3f;
        static constexpr float BRICKS_TEX_SCALE = 0.5f;
        static constexpr float PAD_TEX_SCALE    = 0.35f;

        static constexpr SDL_FRect BALL_TEX  = {ball.x, ball.y, ball.w, ball.h};
        static constexpr SDL_FRect PAD1_TEX  = {pad.x, pad.y, pad.w, pad.h};
        static constexpr SDL_FRect PAD2_TEX  = {pad.x, pad.y, pad.w, pad.h};
        static constexpr SDL_FRect BRICK_TEX = {brick_red_1.x, brick_red_1.y, brick_red_1.w, brick_red_1.h}; // atlas_bricks_ball.h

        SDL_Texture  *tex{};
        SDL_Renderer *ren{};
        SDL_Window   *win{};

        b2WorldId boxWorld = b2_nullWorldId;
    };
};
// @formatter:on
