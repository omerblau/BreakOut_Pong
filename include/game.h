// This is game.h file

#pragma once
#include "bagel.h"
#include "atlas_bricks_ball.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

    // @formatter:off
using namespace bagel;

namespace game {

    enum class UIScreen {Main, Instructions, GameModes, Players, Playing, COUNT};
    enum class GameMode   { None, FirstGoal, BreakAll };
    enum class PlayerSide { None = 0, Single = 1, Two = 2 };

    using brick_coords = struct {SDL_FRect pos[NUM_BRICK_STATE]{}; int idx = 0; };
    using Transform    = struct { SDL_FPoint p; float angle; };
    using Drawable     = struct {SDL_FRect part; SDL_FPoint size; };
    using ChangePart   = struct {brick_coords coords;};
    using Intent       = struct { bool up, down, tilt_down, tilt_up; };
    using Keys         = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    using Collider     = struct { b2BodyId body; };
    using Scorer       = struct { b2ShapeId s; };
    using IsCollision  = struct {};
    using Breakable    = struct {};
    using Goal         = struct {bool left, right;};
    using Ball         = struct {};

    enum class GameState {
        PLAYING,
        PAUSED,
        LEFT_WIN,
        RIGHT_WIN
    };

    using Falling      = struct { float vy; bool playerCollectSide;}; // constant vertical velocity (px/s) for power ups
    using EnlargePU    = struct {}; // tag – specific effect: enlarge paddle
    using PUtimer = struct { float ballHit; }; // countdown timer for power up effects


    class Game {
    public:
        Game();
        ~Game();
        bool valid() const;
        void run();
        void launch();

    private:
        /// meus
        void showScreen(UIScreen s) const;
        void waitMainLoop();
        [[nodiscard]] bool anyKeyStillDown(const bool *keys, int keyCount) const;
        void handleMainKeys(const bool *keys);
        void handleInstructionsKeys(const bool *keys);
        void handleGameModeKeys(const bool *keys);
        bool handlePlayersKeys(const bool *keys);

        /// systems
        void box_system() const;
        void constraints_system() const;
        void input_system() const;
        void move_system() const;
        void draw_system() const;
        void collision_detector_system() const;

        void enlargePaddle(ent_type pad) const;

        void brick_system() const;
        void score_system();
        void cleanup_collision_system() const;

        /// helpers
        void handle_game_state_input();
        void reset_game();
        void destroy_all_entities();
        void paddle_bounds() const;   // Y-clamp  +  angle-clamp
        void ball_speed_cap() const;  // velocity limiter
        bool poll_quit() const;
        void windowClosedClicked();

        void powerup_move_system() const;

        void power_up_system() const;

        void powerup_collision_system() const;

        void enlarge_timer_system() const;

        /// factories
        void createBall() const;
        void createBrick(const SDL_FPoint &pos, int row) const;
        void createPad(const SDL_FRect&, const SDL_FPoint&, const Keys&) const;
        void pace_frame() const;
        void pace_frame_test(Uint64 &start); //todo: remove before submission

        /// init game
        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;
        void createPads() const;
        void placeBricks() const;
        void createPowerUp(const SDL_FRect &r,const SDL_FPoint& pos) const;





        static constexpr int WIN_WIDTH = 1500;
        static constexpr int WIN_HEIGHT = 1000;
        static constexpr int FPS = 60;

        static constexpr float GAME_FRAME = 1000.f / FPS;
        static constexpr float RAD_TO_DEG = 57.2958f;
        static constexpr float DEG_TO_RAD = 1.0f / RAD_TO_DEG;

        static constexpr int   PAD_Y_MARGIN  = 200;

        static constexpr float SPEED_MULTIPLIER = 0.7f;

        static constexpr float BALL_INIT_MPS = 10.0f * SPEED_MULTIPLIER;       // 3 m/s ≈ 30 px/s
        static constexpr float PAD_MOVE      = 12.0f * SPEED_MULTIPLIER;
        static constexpr float PAD_TILT      = 200.0f * SPEED_MULTIPLIER;
        static constexpr float BALL_MAX_MPS  = 13.0f * SPEED_MULTIPLIER;       // 3 m/s ≈ 30 px/s

        static constexpr float BOX_SCALE        = 100.0f;   // 1 m = 10 px
        static constexpr float BALL_TEX_SCALE   = 0.3f;
        static constexpr float BRICKS_TEX_SCALE = 0.5f;
        static constexpr float PAD_TEX_SCALE    = 0.25f;

        static constexpr int BRICK_W = 78;
        static constexpr int BRICK_H = 135;

        static constexpr float PU_SPEED_PPS = 10.0f * SPEED_MULTIPLIER;  // pixels per second

        static constexpr bool RIGHT_PLAYER_POWERUP = false;
        static constexpr bool LEFT_PLAYER_POWERUP = true;

        SDL_Texture  *tex{};
        SDL_Texture  *bgTex{};
        SDL_Texture  *pauseTex{};
        SDL_Texture  *leftWinTex{};
        SDL_Texture  *rightWinTex{};
        SDL_Texture  *uiTex[static_cast<int>(UIScreen::COUNT)]{};
        SDL_Renderer *ren{};
        SDL_Window   *win{};

        b2WorldId boxWorld = b2_nullWorldId;
        mutable GameState gameState = GameState::PLAYING;
        bool shouldQuit = false;

        UIScreen   ui      = UIScreen::Main;
        GameMode   mode    = GameMode::None;        // remembered at GameModes screen
        PlayerSide players = PlayerSide::None;      // remembered at Players screen
        bool       appQuit = false;   // global kill-switch

    };
};
// @formatter:on
