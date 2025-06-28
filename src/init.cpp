// This is init.cpp file

#include "../include/game.h"

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

namespace game {
    bool Game::valid() const {
        return tex != nullptr && bgTex != nullptr && pauseTex != nullptr &&
               leftWinTex != nullptr && rightWinTex != nullptr;
    }

    // todo: clean this funcion too many repeating steps in checks
    bool Game::prepareWindowAndTexture() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cout << SDL_GetError() << std::endl;
            return false;
        }

        if (!SDL_CreateWindowAndRenderer(
            "Breakout Pong", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
            std::cout << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        bgTex = IMG_LoadTexture(ren, "res/bg.png");
        uiTex[static_cast<int>(UIScreen::Main)] = IMG_LoadTexture(ren, "res/bg_mainMenu.png");
        uiTex[static_cast<int>(UIScreen::Instructions)] = IMG_LoadTexture(ren, "res/bg_instructions.png");
        uiTex[static_cast<int>(UIScreen::GameModes)] = IMG_LoadTexture(ren, "res/bg_GameModeMenu.png");
        uiTex[static_cast<int>(UIScreen::Players)] = IMG_LoadTexture(ren, "res/bg_players.png");

        if (!bgTex) {
            std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }


        SDL_Surface *surf = IMG_Load("res/spritesheet.png");
        if (surf == nullptr) {
            std::cout << SDL_GetError() << std::endl;
            SDL_DestroyTexture(bgTex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }

        tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_DestroySurface(surf);

        if (tex == nullptr) {
            std::cout << SDL_GetError() << std::endl;
            SDL_DestroyTexture(bgTex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }

        // Load pause texture
        pauseTex = IMG_LoadTexture(ren, "res/pause.png");
        if (!pauseTex) {
            std::cerr << "Failed to load pause.png: " << SDL_GetError() << "\n";
            SDL_DestroyTexture(tex);
            SDL_DestroyTexture(bgTex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }

        // Load left win texture
        leftWinTex = IMG_LoadTexture(ren, "res/left_win.png");
        if (!leftWinTex) {
            std::cerr << "Failed to load left_win.png: " << SDL_GetError() << "\n";
            SDL_DestroyTexture(pauseTex);
            SDL_DestroyTexture(tex);
            SDL_DestroyTexture(bgTex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }

        // Load right win texture
        rightWinTex = IMG_LoadTexture(ren, "res/right_win.png");
        if (!rightWinTex) {
            std::cerr << "Failed to load right_win.png: " << SDL_GetError() << "\n";
            SDL_DestroyTexture(leftWinTex);
            SDL_DestroyTexture(pauseTex);
            SDL_DestroyTexture(tex);
            SDL_DestroyTexture(bgTex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return false;
        }

        return true;
    }

    void Game::prepareBoxWorld() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0, 0};
        boxWorld = b2CreateWorld(&worldDef);
    }

    Game::Game() {
        if (!prepareWindowAndTexture())
            return;

        SDL_srand(time(nullptr));

        prepareBoxWorld();
    }

    Game::~Game() {
        for (const auto &i: uiTex)
            if (i)
                SDL_DestroyTexture(i);

        if (b2World_IsValid(boxWorld))
            b2DestroyWorld(boxWorld);
        if (tex != nullptr)
            SDL_DestroyTexture(tex);
        if (bgTex != nullptr)
            SDL_DestroyTexture(bgTex);
        if (pauseTex != nullptr)
            SDL_DestroyTexture(pauseTex);
        if (leftWinTex != nullptr)
            SDL_DestroyTexture(leftWinTex);
        if (rightWinTex != nullptr)
            SDL_DestroyTexture(rightWinTex);
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }
}
