#pragma once
#include "../include/components.h"
#include "bagel.h"
#include "atlas_bricks_ball.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

using namespace bagel;

/**
 * @file game.h
 * @brief Main game logic and state management for BreakOut/Pong.
 *
 * This project implements a modern C++ version of the classic Breakout and Pong games, featuring:
 * - Multiple game modes (Breakout, Pong/First Goal)
 * - Single and two-player support
 * - Power-ups, AI, and advanced collision/physics using Box2D
 * - SDL3 for graphics and input
 *
 * The code is organized using an Entity-Component-System (ECS) architecture for flexibility and scalability.
 */

namespace game {

    /**
     * @enum UIScreen
     * @brief Represents the different UI screens in the game.
     */
    enum class UIScreen   {Main, Instructions, GameModes, Players, Playing, COUNT};
    /**
     * @enum GameMode
     * @brief Represents the different game modes.
     */
    enum class GameMode   {None, FirstGoal, BreakAll };
    /**
     * @enum Players
     * @brief Represents the number of players.
     */
    enum class Players    {None = 0, Single = 1, Two = 2};
    /**
     * @enum GameState
     * @brief Represents the current state of the game.
     */
    enum class GameState  {PLAYING, PAUSED, LEFT_WIN, RIGHT_WIN};
    /**
     * @enum PUKind
     * @brief Represents the different types of power-ups.
     */
    enum class PUKind     {EnlargeSelf, ShrinkEnemy, Coin, ExtraBall};

    /**
     * @class Game
     * @brief Main class for handling game logic, state, and systems.
     */
    class Game {
    public:
        /**
         * @brief Constructs a new Game object.
         */
        Game();
        /**
         * @brief Destroys the Game object.
         */
        ~Game();
        /**
         * @brief Checks if the game is in a valid state.
         * @return true if valid, false otherwise.
         */
        bool valid() const;
        /**
         * @brief Runs the main game loop.
         */
        void run();
        /**
         * @brief Launches the game.
         */
        void launch();

    private:
        void showScreen(UIScreen s) const;
        void waitMainLoop();
        static bool anyKeyStillDown(const bool *keys, int keyCount);
        void handleMainKeys(const bool *keys);
        void handleInstructionsKeys(const bool *keys);
        void handleGameModeKeys(const bool *keys);
        bool handlePlayersKeys(const bool *keys);
        void box_system() const;
        static void constraints_system();
        static void input_system();
        void ai_input_system() const;
        static void move_system();
        void draw_system() const;
        void collision_detector_system() const;
        static void protect_brick_damp_system();
        static void ttl_system();
        void brick_system() const;
        void score_system() const;
        static void cleanup_collision_system();
        void pu_timer_system() const;
        void handleCollisionPair(b2ShapeId sa, b2ShapeId sb) const;
        static ent_type findOpponentOf(ent_type pad);
        void applyPowerUp(ent_type pad, ent_type pu) const;
        void createProtectBricks(bool protectRight) const;
        void updateProtectBricks(bool leftActive) const;
        static void replaceProtectBricks(bool isRightSide);
        void spawnExtraBallAt(const SDL_FPoint &pos, bool isRight) const;
        static SDL_FPoint getPaddlePosition(ent_type pad);
        void enlargePaddle(ent_type pad) const;
        void shrinkPadel(ent_type pad) const;
        void handle_game_state_input(bool &exit_run);
        static void reset_game();
        void create_game() const;
        static void destroy_all_entities();
        static void DestroyBodySafe(b2BodyId b);
        static void paddle_bounds();
        static void ball_speed_cap();
        void windowClosedClicked();
        void createPowerUpRotating(const SDL_FPoint &pos) const;
        static void powerup_move_system();
        static void addSideTag(ent_type e, bool isRight);
        void paddleBackToOG(ent_type pad) const;
        static void pace_frame();
        static constexpr const SDL_FRect& spriteFor(const PUKind k) {
            for (auto &[kind, rect]: spriteByKind)
                if (kind == k) return rect;
            return POWERUP_ENLARGE;
        }
        void createBall() const;
        void createBrick(const SDL_FPoint &pos, int row, bool isRight) const;
        void createPad(const SDL_FRect&, const SDL_FPoint&, const Keys&, bool isRight) const;
        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;
        void createPads() const;
        void placeBricks() const;
        void createPowerUp(const SDL_FRect &r,const SDL_FPoint& pos, PUKind kind) const;
        static constexpr int    WIN_WIDTH            = 1500;
        static constexpr int    WIN_HEIGHT           = 1000;
        static constexpr int    FPS                  = 60;
        static constexpr float  GAME_FRAME           = 1000.f / FPS;
        static constexpr float  RAD_TO_DEG           = 57.2958f;
        static constexpr float  DEG_TO_RAD           = 1.0f / RAD_TO_DEG;
        static constexpr float  BOX_SCALE            = 100.0f;
        static constexpr float  BALL_TEX_SCALE       = 0.3f;
        static constexpr float  BRICKS_TEX_SCALE     = 0.5f;
        static constexpr float  PAD_TEX_SCALE        = 0.25f;
        static constexpr float  SPEED_MULTIPLIER     = 0.7f;
        static constexpr float  BALL_INIT_MPS        = 14.0f * SPEED_MULTIPLIER;
        static constexpr float  BALL_MAX_MPS         = 20.0f * SPEED_MULTIPLIER;
        static constexpr float  BALL_MIN_MPS         = 10.0f * SPEED_MULTIPLIER;
        static constexpr int    PAD_Y_MARGIN         = 200;
        static constexpr float  VIS_MARGIN_M         = 4.0f / BOX_SCALE;
        static constexpr float  PAD_MOVE             = 12.0f * SPEED_MULTIPLIER;
        static constexpr float  PAD_TILT             = 200.0f * SPEED_MULTIPLIER;
        static constexpr int    BRICK_COLS           = 3;
        static constexpr int    BRICK_ROWS           = 14;
        static constexpr int    BRICK_W              = 78;
        static constexpr int    BRICK_H              = 135;
        static constexpr float  PU_SPEED_PPS         = 4.0f * SPEED_MULTIPLIER;
        static constexpr bool   RIGHT_PLAYER_POWERUP = false;
        static constexpr bool   LEFT_PLAYER_POWERUP  = true;
        static constexpr int    HITS_NUM_PU_CREATION = 3;
        static constexpr float  WALL_GAP             = 200.0f;
        static constexpr Keys   RIGHT_KEYS = {
            SDL_SCANCODE_UP,
            SDL_SCANCODE_DOWN,
            SDL_SCANCODE_RIGHT,
            SDL_SCANCODE_LEFT
        };
        static constexpr Keys   LEFT_KEYS = {
            SDL_SCANCODE_W,
            SDL_SCANCODE_S,
            SDL_SCANCODE_D,
            SDL_SCANCODE_A
        };
        static constexpr auto spriteByKind = {
            std::pair{PUKind::EnlargeSelf, POWERUP_ENLARGE},
            std::pair{PUKind::ShrinkEnemy, POWERUP_SHRINK},
            std::pair{PUKind::Coin, POWERUP_COIN},
            std::pair{PUKind::ExtraBall, POWERUP_4_COORDS}
        };
        SDL_Texture       *tex{};
        SDL_Texture       *bgTex{};
        SDL_Texture       *pauseTex{};
        SDL_Texture       *leftWinTex{};
        SDL_Texture       *rightWinTex{};
        SDL_Texture       *uiTex[static_cast<int>(UIScreen::COUNT)]{};
        SDL_Renderer      *ren{};
        SDL_Window        *win{};
        b2WorldId         boxWorld   = b2_nullWorldId;
        mutable GameState gameState  = GameState::PLAYING;
        bool              shouldQuit = false;
        UIScreen          ui         = UIScreen::Main;
        GameMode          mode       = GameMode::None;
        Players           players    = Players::None;
        bool              appQuit    = false;
    };
}

