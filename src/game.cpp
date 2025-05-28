#pragma once
#include "../include/game.h"

 namespace game {
     bool Game::prepareWindow()
     {
         if (!SDL_Init(SDL_INIT_VIDEO)) {
             std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
             return false;
         }

         if (!SDL_CreateWindowAndRenderer("Alley Cat", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
             std::cerr << "SDL_CreateWindowAndRenderer Error: " << SDL_GetError() << "\n";
             return false;
         }

         // // Load background texture
         // bgTex = IMG_LoadTexture(ren, "res/cat_bg.png");
         // if (!bgTex) {
         //     std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
         //     return false;
         // }


         return true;
     }

     void Game::prepareBoxWorld()
     {
         b2WorldDef worldDef = b2DefaultWorldDef();
         worldDef.gravity = {0.0f, 18.0f};
         boxWorld = b2CreateWorld(&worldDef);
     }
 }