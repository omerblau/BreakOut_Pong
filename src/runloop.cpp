// This is runloop.cpp file

#include "../include/game.h"

#include <SDL3/SDL.h>

namespace game {
    bool Game::poll_quit() const {
        // This is now just a flag check, actual polling happens in handle_game_state_input
        return false;
    }

    void Game::handle_game_state_input() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT ||
                (e.type == SDL_EVENT_KEY_DOWN &&
                 e.key.scancode == SDL_SCANCODE_ESCAPE)) {
                // Set a quit flag that will be checked by poll_quit
                const_cast<Game *>(this)->shouldQuit = true;
                continue;
            }

            if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (gameState) {
                    case GameState::PLAYING:
                        if (e.key.scancode == SDL_SCANCODE_P) {
                            gameState = GameState::PAUSED;
                        }
                        break;

                    case GameState::PAUSED:
                        if (e.key.scancode == SDL_SCANCODE_P) {
                            gameState = GameState::PLAYING;
                        } else if (e.key.scancode == SDL_SCANCODE_N) {
                            reset_game();
                            gameState = GameState::PLAYING;
                        }
                        break;

                    case GameState::LEFT_WIN:
                    case GameState::RIGHT_WIN:
                        if (e.key.scancode != SDL_SCANCODE_ESCAPE) {
                            reset_game();
                            gameState = GameState::PLAYING;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }

    void Game::windowClosedClicked() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                appQuit = true;
        }
    }

    void Game::pace_frame() const {
        static Uint32 frameStart = SDL_GetTicks();
        const Uint64 frameEnd = SDL_GetTicks();
        const Uint64 elapsed = frameEnd - frameStart;
        if (elapsed < static_cast<Uint64>(GAME_FRAME))
            SDL_Delay(static_cast<Uint32>(GAME_FRAME - static_cast<float>(elapsed)));
        frameStart += static_cast<Uint64>(GAME_FRAME); // schedule next frame
    }

    void Game::run() {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

        bool quit = false;
        bool gameExitToMenu = false;
        while (!gameExitToMenu && !appQuit) {
            World::step();
            const_cast<Game *>(this)->handle_game_state_input();

            // Only run game systems when playing
            if (gameState == GameState::PLAYING) {
                input_system();
                move_system();
                box_system();
                constraints_system();
                collision_detector_system();
                brick_system();
                const_cast<Game *>(this)->score_system();
                cleanup_collision_system();
            }
            draw_system();

            pace_frame();
            windowClosedClicked();
            quit = shouldQuit;
        }
    }
}
