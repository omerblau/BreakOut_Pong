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
    using Transform    = struct { SDL_FPoint p; float a; };
    using Drawable     = struct {SDL_FRect part; SDL_FPoint size; };
    using ChangePart   = struct {brick_coords coords;};
    using Intent       = struct { bool up, down, tilt_down, tilt_up; };
    using Keys         = struct { SDL_Scancode up, down, tilt_down, tilt_up; };
    using Collider     = struct { b2BodyId b; };
    using Scorer       = struct { b2ShapeId s; };
    using IsCollision  = struct {};
    using Breakable    = struct {};
    using Goal         = struct {bool left, right;};

    class Game {
    public:
        Game();
        ~Game();
        [[nodiscard]] bool valid() const; //todo: remove this if we never use later (moshe had it)
        void run();
        void launch();

    private:
        /// meus
        void showScreen(UIScreen s) const;
        void waitMainLoop();
        void refreshKeyState();
        [[nodiscard]] bool anyKeyStillDown() const;
        void   handleMainKeys();
        void   handleInstructionsKeys();
        void   handleGameModeKeys();
        bool   handlePlayersKeys();

        /// systems
        void box_system() const;
        void constraints_system() const;
        void input_system() const;
        void move_system() const;
        void draw_system() const;
        void collision_detector_system() const;
        void brick_system() const;
        void score_system() const;

        /// helpers
        void paddle_bounds() const;
        void ball_speed_cap() const;
        void windowClosedClicked();

        /// factories
        void createBall() const;
        void createBrick(const SDL_FPoint &pos, int row) const;
        void createPad(const SDL_FRect&, const SDL_FPoint&, const Keys&) const;
        void pace_frame() const;


        /// init game
        bool prepareWindowAndTexture();
        void prepareBoxWorld();
        void prepareWalls() const;
        void createPads() const;
        void placeBricks() const;

        static constexpr int WIN_WIDTH = 1500;
        static constexpr int WIN_HEIGHT = 1000;
        static constexpr int FPS = 60;

        static constexpr float GAME_FRAME = 1000.f / FPS;
        static constexpr float RAD_TO_DEG = 57.2958f;
        static constexpr float DEG_TO_RAD = 1.0f / RAD_TO_DEG;

        static constexpr int   PAD_Y_MARGIN  = 200;
        static constexpr float PAD_MOVE      = 5.0f;
        static constexpr float PAD_TILT      = 12.0f;
        static constexpr float BALL_INIT_MPS = 3.0f;       // 3 m/s ≈ 30 px/s

        static constexpr float BOX_SCALE        = 10.0f;   // 1 m = 10 px
        static constexpr float BALL_TEX_SCALE   = 0.3f;
        static constexpr float BRICKS_TEX_SCALE = 0.5f;
        static constexpr float PAD_TEX_SCALE    = 0.35f;

        static constexpr int BRICK_W = 78;
        static constexpr int BRICK_H = 135;

        SDL_Texture  *tex{};
        SDL_Texture  *bgTex{};
        SDL_Texture  *uiTex[static_cast<int>(UIScreen::COUNT)]{};
        SDL_Renderer *ren{};
        SDL_Window   *win{};

        b2WorldId boxWorld = b2_nullWorldId;

        UIScreen    ui        = UIScreen::Main;
        GameMode mode      = GameMode::None;        // remembered at GameModes screen
        PlayerSide players   = PlayerSide::None;      // remembered at Players screen
        const bool* keyState  = nullptr;   // updated once per frame
        int         keyCount  = 0;
        bool        appQuit   = false;   // global kill-switch

    };
};
// @formatter:on
