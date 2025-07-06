// This is runloop.cpp file

#include "../include/game.h"

#include <SDL3/SDL.h>

namespace game {
    void Game::handle_game_state_input(bool &exit_run) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT ||
                (e.type == SDL_EVENT_KEY_DOWN &&
                 e.key.scancode == SDL_SCANCODE_ESCAPE)) {
                appQuit = true;
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
                            exit_run = true;
                            gameState = GameState::PLAYING;
                        }
                        break;

                    case GameState::LEFT_WIN:
                    case GameState::RIGHT_WIN:
                        if (e.key.scancode != SDL_SCANCODE_ESCAPE) {
                            reset_game();
                            exit_run = true;
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

    void Game::pace_frame() {
        static Uint32 frameStart = SDL_GetTicks();
        const Uint64 frameEnd = SDL_GetTicks();
        const Uint64 elapsed = frameEnd - frameStart;
        if (elapsed < static_cast<Uint64>(GAME_FRAME))
            SDL_Delay(static_cast<Uint32>(GAME_FRAME - static_cast<float>(elapsed)));
        frameStart += static_cast<Uint64>(GAME_FRAME); // schedule next frame
    }

    void Game::run() {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

        bool exit_run = false;
        while (!appQuit) {
            World::step();
            handle_game_state_input(exit_run);
            if (exit_run) break;

            if (gameState == GameState::PLAYING) {
                input_system();
                ai_input_system();
                move_system();
                box_system();
                constraints_system();
                protect_brick_damp_system();
                collision_detector_system();
                brick_system();
                ttl_system();
                powerup_move_system();
                pu_timer_system();
                score_system();
                cleanup_collision_system();
            }
            draw_system();
            pace_frame();
        }
    }
}


