// This is menu.cpp file

#include "../include/game.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <algorithm>

namespace game {
    bool Game::anyKeyStillDown(const bool *keys, const int keyCount) const {
        for (int i = 0; i < keyCount; ++i)
            if (keys[i]) return true;
        return false;
    }

    void Game::handleMainKeys(const bool *keys) {
        if (keys[SDL_SCANCODE_1])
            ui = UIScreen::GameModes;
        else if (keys[SDL_SCANCODE_2])
            ui = UIScreen::Instructions;
    }

    void Game::handleInstructionsKeys(const bool *keys) {
        if (keys[SDL_SCANCODE_1])
            ui = UIScreen::Main;
    }

    void Game::handleGameModeKeys(const bool *keys) {
        if (keys[SDL_SCANCODE_1]) {
            mode = GameMode::FirstGoal;
            ui = UIScreen::Players;
        } else if (keys[SDL_SCANCODE_2]) {
            mode = GameMode::BreakAll;
            ui = UIScreen::Players;
        } else if (keys[SDL_SCANCODE_3]) {
            ui = UIScreen::Main;
        }
    }

    bool Game::handlePlayersKeys(const bool *keys) {
        if (keys[SDL_SCANCODE_1]) {
            players = PlayerSide::Single;
            return true;
        }
        if (keys[SDL_SCANCODE_2]) {
            players = PlayerSide::Two;
            ui = UIScreen::Main;
            return true;
        }
        if (keys[SDL_SCANCODE_3])
            ui = UIScreen::GameModes;
        else if (keys[SDL_SCANCODE_M])
            ui = UIScreen::Main;
        return false;
    }

    void Game::showScreen(UIScreen s) const {
        // 1. Fetch texture size
        float imgW{}, imgH{};
        if (!uiTex[static_cast<int>(s)] ||
            !SDL_GetTextureSize(uiTex[static_cast<int>(s)], &imgW, &imgH)) {
            std::cerr << "showScreen: texture missing or size query failed for state "
                    << static_cast<int>(s) << " – " << SDL_GetError() << '\n';
            return;
        }

        // 2. “Cover” scale so the window fills without distortion
        const float scale = std::max(WIN_WIDTH / imgW,
                                     WIN_HEIGHT / imgH);

        const SDL_FRect dst{
            (WIN_WIDTH - imgW * scale) * 0.5f,
            (WIN_HEIGHT - imgH * scale) * 0.5f,
            imgW * scale,
            imgH * scale
        };

        // 3. Render
        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, uiTex[static_cast<int>(s)], nullptr, &dst);
        SDL_RenderPresent(ren);
    }

    void Game::waitMainLoop() {
        bool waitKeyRelease = false;

        while (!appQuit) {
            SDL_PumpEvents();
            int keyCount = 0;
            const bool *keys = SDL_GetKeyboardState(&keyCount);

            if (waitKeyRelease) {
                if (anyKeyStillDown(keys, keyCount)) {
                    showScreen(ui);
                    pace_frame();
                    continue;
                }
                waitKeyRelease = false;
            }

            const UIScreen prev = ui;
            bool startGame = false; // s

            switch (ui) {
                case UIScreen::Main: handleMainKeys(keys);
                    break;
                case UIScreen::Instructions: handleInstructionsKeys(keys);
                    break;
                case UIScreen::GameModes: handleGameModeKeys(keys);
                    break;
                case UIScreen::Players: startGame = handlePlayersKeys(keys);
                    break;
                default:
                    break;
            }

            if (ui != prev)
                waitKeyRelease = true;

            showScreen(ui);
            pace_frame();

            if (startGame)
                return;

            if (keys[SDL_SCANCODE_ESCAPE])
                appQuit = true;
            windowClosedClicked();
        }
    }

    void Game::launch() {
        while (!appQuit) {
            waitMainLoop();
            if (appQuit)
                break;

            run();
        }
    }
}
