#pragma once
#include "bagel.h"
#include "../include/game.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include <iostream>
#include "components.h"

using namespace bagel;

namespace game {

    class Game
    {
    public:
        Game();
        ~Game();

        bool valid() const;
        void run();

    private:
        bool prepareWindow();
        void prepareBoxWorld();
        void createBallEntity();
        void createPadEntity();
        void createWalls();
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
        };
    };
}

