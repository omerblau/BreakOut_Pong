// This is the newgame.h file

#pragma once
#include "bagel.h"
#include "../res/atlas_bricks_ball.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

using namespace bagel;

namespace game {

    // @formatter:off
    using Transform = struct { SDL_FPoint p; float a; };
    using Drawable = struct { SDL_FRect part; SDL_FPoint size; };
    using Intent = struct { bool up, down; };
    using Keys = struct { SDL_Scancode up, down; };
    using Collider = struct { b2BodyId b; };
    using Scorer = struct { b2ShapeId s; };
    // @formatter:on

    class Game {
    public:
        Game();
        ~Game();
        bool valid() const;
        void run();

    private:
        void box_system() const;
        void draw_system() const;

        void createBall() const;

        void createBrick(const SDL_FPoint &pos) const;

        void placeBricks() const;

        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;

        static constexpr int WIN_WIDTH = 1500;
        static constexpr int WIN_HEIGHT = 1000;
        static constexpr int FPS = 60;

        static constexpr float GAME_FRAME = 1000.f / FPS;
        static constexpr float RAD_TO_DEG = 57.2958f;

        static constexpr float BOX_SCALE      = 10.0f;   // 1 m = 10 px
        static constexpr float BALL_INIT_MPS  = 3.0f;    // 3 m/s ≈ 30 px/s

        static constexpr float BALL_TEX_SCALE = 0.5f;
        static constexpr float PAD_TEX_SCALE = 0.25f;

        static constexpr SDL_FRect BALL_TEX = {ball.x, ball.y, ball.w, ball.h};
        static constexpr SDL_FRect PAD1_TEX = {pad.x, pad.y, pad.w, pad.h};
        static constexpr SDL_FRect PAD2_TEX = {pad.x, pad.y, pad.w, pad.h};
        static constexpr SDL_FRect DOTS_TEX = {296, 20, 24, 24};
        static constexpr SDL_FRect BRICK_TEX = {brick_red_1.x, brick_red_1.y, brick_red_1.w, brick_red_1.h}; // atlas_bricks_ball.h


        SDL_Texture *tex{};
        SDL_Renderer *ren{};
        SDL_Window *win{};

        b2WorldId boxWorld = b2_nullWorldId;
    };
};
