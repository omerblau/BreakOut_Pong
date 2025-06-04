#pragma once
#include "bagel.h"
#include "../include/game.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include <iostream>
#include "components.h"
#include "../res/atlas_bricks_ball.h"

using namespace bagel;

namespace game {

    class Game
    {
    public:
        Game();
        ~Game();

        bool valid() const;
        void run();

        void box_system() const;

    private:
        bool prepareWindow();
        void prepareBoxWorld();

        void createBall() const;

        void createBallEntity();
        void createPadEntity();
        void createWalls();

        void move_system() const;

        void draw_system() const;

        void createBricks();

        void inputSystem();
        void moveSystem();
        void drawSystem();
        void scoreSystem(); //maybe not needed in this game
        void boxSystem(float deltaTime);

        SDL_Renderer* ren;
        SDL_Window* win;

        b2WorldId boxWorld = b2_nullWorldId;

        static constexpr int    WIN_WIDTH = 1280;
        static constexpr int    WIN_HEIGHT = 800;
        static constexpr int	FPS = 60;


        static constexpr float	GAME_FRAME = 1000.f/FPS;
        static constexpr float	RAD_TO_DEG = 57.2958f;

        static constexpr float	BOX_SCALE = 10;
        static constexpr float	BALL_TEX_SCALE = 0.5f;
        static constexpr float	PAD_TEX_SCALE = 0.25f;

        static constexpr SDL_FRect BALL_TEX = {ball.x, ball.y, ball.w, ball.h};


        SDL_Renderer* ren = nullptr;
        SDL_Window* win = nullptr;
        SDL_Texture* tex = nullptr;
        b2WorldId boxWorld = b2_nullWorldId;
        };
    };
}

