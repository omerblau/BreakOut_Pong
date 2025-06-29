// This is game.h file

#pragma once
#include <array>

#include "bagel.h"
#include "atlas_bricks_ball.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

    // @formatter:off
using namespace bagel;

namespace game {

    enum class UIScreen   {Main, Instructions, GameModes, Players, Playing, COUNT};
    enum class GameMode   {None, FirstGoal, BreakAll };
    enum class Players    {None = 0, Single = 1, Two = 2};
    enum class GameState  {PLAYING, PAUSED, LEFT_WIN, RIGHT_WIN};
    enum class PaddleSide {Left, Right};

    using brick_coords = struct {SDL_FRect pos[NUM_BRICK_STATE]{}; int idx = 0; };
    using Transform    = struct { SDL_FPoint p; float angle; };
    using Drawable     = struct {SDL_FRect part; SDL_FPoint size; };
    using ChangePart   = struct {brick_coords coords;};
    using Intent       = struct { bool up, down, tilt_down, tilt_up; };
    using Keys         = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    using Collider     = struct { b2BodyId body; };
    using Scorer       = struct { b2ShapeId s; };
    using Goal         = struct {bool left, right;};
    using AI           = struct {
                            float targetY = -1.0f;
                            int tiltFramesRemaining = 0;
                            int tiltDirection = 0;  // -1 = up, 1 = down
                            int cooldownFrames = 1;
                         };
    using IsCollision  = struct {};
    using Breakable    = struct {};
    using Ball         = struct {};


    enum class PUKind {
        EnlargeSelf,
        ShrinkEnemy,
        Coin,
        ExtraBall
    };

    inline constexpr std::array spriteByKind{
        std::pair{PUKind::EnlargeSelf, POWERUP_ENLARGE},
        std::pair{PUKind::ShrinkEnemy, POWERUP_SHRINK},
        std::pair{PUKind::Coin, POWERUP_COIN},
        std::pair{PUKind::ExtraBall, POWERUP_4_COORDS}
    };

    using Falling      = struct { float vy; bool playerCollectSide;}; // constant vertical velocity (px/s) for power ups
    using EnlargePU    = struct {}; // tag – specific effect: enlarge paddle
    using PUtimer = struct { float hitsLeft; }; // countdown timer for power up effects
    using TagLeft = struct {}; // tag for left player power up
    using TagRight = struct {}; // tag for right player power up
    using PU_EnlargeSelf = struct {};
    using PU_ShrinkEnemy = struct {};
    using PU_Coin = struct {};
    using PU_ExtraBall = struct {};



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

        void ai_input_system() const;

        void move_system() const;
        void draw_system() const;
        void collision_detector_system() const;

        void handleCollisionPair(b2ShapeId sa, b2ShapeId sb) const;

        ent_type findOpponentOf(ent_type pad) const;

        void applyPowerUp(ent_type pad, ent_type pu) const;

        void spawnExtraBallAt(const SDL_FPoint &pos, bool isRight) const;

        SDL_FPoint getPaddlePosition(ent_type pad) const;

        void enlargePaddle(ent_type pad) const;

        void shrinkPadel(ent_type pad) const;

        void brick_system() const;


        void score_system();
        void cleanup_collision_system() const;

        /// helpers
        void handle_game_state_input(bool &exit_run);
        void reset_game();

        void create_game() const;

        void destroy_all_entities();
        void paddle_bounds() const;
        void ball_speed_cap() const;
        bool poll_quit() const;
        void windowClosedClicked();

        void createPowerUpRotating(const SDL_FPoint &pos) const;

        void powerup_move_system();

        void power_up_system() const;

        void powerup_collision_system() const;

        void enlarge_timer_system() const;

        static void addSideTag(ent_type e, bool isRight) ;


        void pu_timer_system() const;

        void paddleBackToOG(ent_type pad) const;

        /// factories
        void createBall() const;

        void createBrick(const SDL_FPoint &pos, int row, bool isRight) const;
        void createPad(const SDL_FRect&, const SDL_FPoint&, const Keys&, PaddleSide side, bool isRight) const;
        void pace_frame() const;

        /// init game
        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;
        void createPads() const;
        void placeBricks() const;
        void createPowerUp(const SDL_FRect &r,const SDL_FPoint& pos, PUKind kind) const;





        static constexpr int WIN_WIDTH = 1500;
        static constexpr int WIN_HEIGHT = 1000;
        static constexpr int FPS = 60;

        static constexpr float GAME_FRAME = 1000.f / FPS;
        static constexpr float RAD_TO_DEG = 57.2958f;
        static constexpr float DEG_TO_RAD = 1.0f / RAD_TO_DEG;

        static constexpr int   PAD_Y_MARGIN  = 200;

        static constexpr float SPEED_MULTIPLIER = 1.0f;

        static constexpr float BALL_INIT_MPS = 10.0f * SPEED_MULTIPLIER;       // 3 m/s ≈ 30 px/s
        static constexpr float PAD_MOVE      = 12.0f * SPEED_MULTIPLIER;
        static constexpr float PAD_TILT      = 200.0f * SPEED_MULTIPLIER;
        static constexpr float BALL_MAX_MPS  = 23.0f * SPEED_MULTIPLIER;       // 3 m/s ≈ 30 px/s

        static constexpr float BOX_SCALE        = 100.0f;   // 1 m = 10 px
        static constexpr float BALL_TEX_SCALE   = 0.3f;
        static constexpr float BRICKS_TEX_SCALE = 0.5f;
        static constexpr float PAD_TEX_SCALE    = 0.25f;

        static constexpr int BRICK_W = 78;
        static constexpr int BRICK_H = 135;

        static constexpr float PU_SPEED_PPS = 4.0f * SPEED_MULTIPLIER;  // pixels per second

        static constexpr bool RIGHT_PLAYER_POWERUP = false;
        static constexpr bool LEFT_PLAYER_POWERUP = true;


        static constexpr int HITS_NUM_PU_CREATION = 3;


        static constexpr Keys RIGHT_KEYS = {
            SDL_SCANCODE_UP,
            SDL_SCANCODE_DOWN,
            SDL_SCANCODE_RIGHT,
            SDL_SCANCODE_LEFT
        };

        static constexpr Keys LEFT_KEYS = {
            SDL_SCANCODE_W,
            SDL_SCANCODE_S,
            SDL_SCANCODE_D,
            SDL_SCANCODE_A
        };

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
        Players players = Players::None;      // remembered at Players screen
        bool       appQuit = false;   // global kill-switch

    };
};
// @formatter:on
