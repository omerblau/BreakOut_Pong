// This is the newgame.cpp file

#include "../include/newgame.h"

#include "../include/Pong.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include "../lib/box2d/src/body.h"

using namespace std;

#include "../include/bagel.h"
using namespace bagel;

namespace game {
    bool Game::valid() const {
        return tex != nullptr;
    }

    void Game::createBall() const {
        b2BodyDef ballBodyDef = b2DefaultBodyDef();
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.fixedRotation = false;
        ballBodyDef.position = {WIN_WIDTH / 2 / BOX_SCALE, WIN_HEIGHT / 2 / BOX_SCALE};

        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.enableSensorEvents = true;
        ballShapeDef.density = 1;
        ballShapeDef.material.friction = 0;
        ballShapeDef.material.restitution = 1.0f;
        b2Circle ballCircle = {0, 0, BALL_TEX.w * BALL_TEX_SCALE / BOX_SCALE / 2};

        b2BodyId ballBody = b2CreateBody(boxWorld, &ballBodyDef);
        b2CreateCircleShape(ballBody, &ballShapeDef, &ballCircle);

        float xs = SDL_randf() / 2 + .25f;
        if (SDL_rand(2)) xs = -xs;
        float ys = SDL_sqrtf(1 - xs * xs);
        if (SDL_rand(2)) ys = -ys;
        b2Body_SetLinearVelocity(ballBody, {xs * BALL_INIT_MPS, ys * BALL_INIT_MPS});

        Entity ballEntity = Entity::create();
        ballEntity.addAll(
            Transform{{}, 0},
            Drawable{{BALL_TEX}, {BALL_TEX.w * BALL_TEX_SCALE, BALL_TEX.h * BALL_TEX_SCALE}},
            Collider{ballBody}
        );
        b2Body_SetUserData(ballBody, new ent_type{ballEntity.entity()});
    }

    bool Game::prepareWindowAndTexture() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return false;
        }

        if (!SDL_CreateWindowAndRenderer(
            "Breakout Pong", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return false;
        }

        SDL_Surface *surf = IMG_Load("res/spritesheet.png");
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return false;
        }

        tex = SDL_CreateTextureFromSurface(ren, surf);
        if (tex == nullptr) {
            cout << SDL_GetError() << endl;
            return false;
        }

        SDL_DestroySurface(surf);
        return true;
    }

    void Game::prepareBoxWorld() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0, 0};
        boxWorld = b2CreateWorld(&worldDef);
    }

    void Game::prepareWalls() const    {
        /* ---------- 1. Body & shape templates ---------- */
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type      = b2_staticBody;      // every wall is static

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density    = 1;                // irrelevant for static bodies
        shapeDef.isSensor   = false;            // ← hard surface
        shapeDef.enableSensorEvents = false;

        /* ---------- 2. Small helper to spawn one wall ---------- */
        auto makeWall = [&](float cx, float cy,
                            float hx, float hy)
        {
            b2Polygon box = b2MakeBox(hx, hy);
            bodyDef.position = { cx, cy };

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);
        };

        /* ---------- 3. Dimensions in physics-metres ---------- */
        constexpr float W = WIN_WIDTH  / BOX_SCALE;   // play-field width
        constexpr float H = WIN_HEIGHT / BOX_SCALE;   // play-field height
        constexpr float T = 1.0f;                     // wall half-thickness

        /* ---------- 4. Build the four walls ---------- */
        // ⬆ Top
        makeWall(W * 0.5f, -T,          /* centre */ W * 0.5f, T);

        // ⬇ Bottom
        makeWall(W * 0.5f, H + T,       /* centre */ W * 0.5f, T);

        // ⬅ Left
        makeWall(-T,        H * 0.5f,   /* centre */ T,        H * 0.5f);

        // ➡ Right
        makeWall(W + T,     H * 0.5f,   /* centre */ T,        H * 0.5f);
    }

    void Game::box_system() const {
        static const Mask mask = MaskBuilder()
                .set<Collider>()
                .set<Transform>()
                .build();
        static constexpr float BOX2D_STEP = 1.f / FPS;

        b2World_Step(boxWorld, BOX2D_STEP, 4);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                b2Transform t = b2Body_GetTransform(World::getComponent<Collider>(e).b);
                World::getComponent<Transform>(e) = {
                    {t.p.x * BOX_SCALE, t.p.y * BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
            }
        }
    }

    void Game::draw_system() const {
        static const Mask mask = MaskBuilder()
                .set<Transform>()
                .set<Drawable>()
                .build();

        SDL_RenderClear(ren);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                const auto &d = World::getComponent<Drawable>(e);
                const auto &t = World::getComponent<Transform>(e);

                const SDL_FRect dst = {
                    t.p.x - d.size.x / 2,
                    t.p.y - d.size.y / 2,
                    d.size.x, d.size.y
                };

                SDL_RenderTextureRotated(
                    ren, tex, &d.part, &dst, t.a,
                    nullptr, SDL_FLIP_NONE);
            }
        }

        SDL_RenderPresent(ren);
    }

    Game::Game() {
        if (!prepareWindowAndTexture())
            return;
        SDL_srand(time(nullptr));

        prepareBoxWorld();
        prepareWalls();
        createBall();
        // createPad(PAD1_TEX, {50, WIN_HEIGHT / 2}, {SDL_SCANCODE_W, SDL_SCANCODE_S});
        // createPad(PAD2_TEX, {WIN_WIDTH - 50, WIN_HEIGHT / 2}, {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN});
    }

    Game::~Game() {
        if (b2World_IsValid(boxWorld))
            b2DestroyWorld(boxWorld);
        if (tex != nullptr)
            SDL_DestroyTexture(tex);
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    void Game::run() {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        auto start = SDL_GetTicks();
        bool quit = false;

        // InputSystem is;
        while (!quit) {
            //is.updateEntities();
            //first updateEntities() for all systems

            //is.update();
            //then update() for all systems

            World::step();
            //finally World::step() to clear added() array

            //input_system();
            // move_system();
            box_system();
            // score_system();
            draw_system();

            const auto end = SDL_GetTicks();
            if (const auto elapsed = end - start) {
                SDL_Delay(static_cast<Uint32>(GAME_FRAME - static_cast<float>(elapsed)));
            }
            start += static_cast<Uint64>(GAME_FRAME);

            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT)
                    quit = true;
                else if ((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE))
                    quit = true;
            }
        }
    }
}
