// This is the newgame.cpp file

#include "../include/game.h"

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
        ballShapeDef.enableContactEvents = true;
        ballShapeDef.density = 1;
        ballShapeDef.material.friction = 0;
        ballShapeDef.material.restitution = 1.0f;
        b2Circle ballCircle = {0, 0, BALL_COORDS.w * BALL_TEX_SCALE / BOX_SCALE / 2};

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
            Drawable{BALL_COORDS, {BALL_COORDS.w * BALL_TEX_SCALE, BALL_COORDS.h * BALL_TEX_SCALE}},
            Collider{ballBody}
        );
        b2Body_SetUserData(ballBody, new ent_type{ballEntity.entity()});
    }

    void Game::createPad(const SDL_FRect &r, const SDL_FPoint &p, const Keys &k) const {
        b2BodyDef padBodyDef = b2DefaultBodyDef();
        padBodyDef.type = b2_kinematicBody;
        padBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        const b2BodyId padBody = b2CreateBody(boxWorld, &padBodyDef);

        constexpr float angleRad = 90.0f / RAD_TO_DEG;
        const b2Rot rot = {std::cos(angleRad), std::sin(angleRad)};
        b2Body_SetTransform(padBody, padBodyDef.position, rot);

        b2ShapeDef padShapeDef = b2DefaultShapeDef();
        padShapeDef.density = 0;

        const b2Polygon padBox = b2MakeBox(
            r.w * PAD_TEX_SCALE / BOX_SCALE / 2,
            r.h * PAD_TEX_SCALE / BOX_SCALE / 2);
        b2CreatePolygonShape(padBody, &padShapeDef, &padBox);
        Entity padEntity = Entity::create();
        padEntity.addAll(
            Transform{{}, 0},
            Drawable{r, {r.w * PAD_TEX_SCALE, r.h * PAD_TEX_SCALE}},
            Collider{padBody},
            Intent{},
            k
        );
        b2Body_SetUserData(padBody, new ent_type{padEntity.entity()});
    }

    void Game::createPads() const {
        createPad(PAD_COORDS, {PAD_Y_MARGIN, WIN_HEIGHT / 2}, {
                      SDL_SCANCODE_W,
                      SDL_SCANCODE_S,
                      SDL_SCANCODE_D,
                      SDL_SCANCODE_A
                  });
        createPad(PAD_COORDS, {WIN_WIDTH - PAD_Y_MARGIN, WIN_HEIGHT / 2}, {
                      SDL_SCANCODE_UP,
                      SDL_SCANCODE_DOWN,
                      SDL_SCANCODE_LEFT,
                      SDL_SCANCODE_RIGHT
                  });
    }

    void Game::createBrick(const SDL_FPoint &pos, int row) const {
        // physics body
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_staticBody;
        def.position = {pos.x / BOX_SCALE, pos.y / BOX_SCALE};

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1;

        b2Polygon box = b2MakeBox(
            BRICK_W * BRICKS_TEX_SCALE / BOX_SCALE / 2,
            BRICK_H * BRICKS_TEX_SCALE / BOX_SCALE / 2
        );

        b2BodyId body = b2CreateBody(boxWorld, &def);
        b2CreatePolygonShape(body, &shapeDef, &box);

        brick_coords coords;
        switch (row % 4) {
            case 0:
                coords = {RED_BRICK_COORDS[0], RED_BRICK_COORDS[1], 0};
                break;
            case 1:
                coords = {ORANGE_BRICK_COORDS[0], ORANGE_BRICK_COORDS[1], 0};
                break;
            case 2:
                coords = {GREEN_BRICK_COORDS[0], GREEN_BRICK_COORDS[1], 0};
                break;
            case 3:
                coords = {YELLOW_BRICK_COORDS[0], YELLOW_BRICK_COORDS[1], 0};
                break;
            default:
                break;
        }

        Entity brickEntity = Entity::create();
        brickEntity.addAll(
            Transform{pos, 0},
            Drawable{coords.pos[0], {BRICK_W * BRICKS_TEX_SCALE, BRICK_H * BRICKS_TEX_SCALE}},
            ChangePart{coords},
            Collider{body},
            Breakable{}
        );
        b2Body_SetUserData(body, new ent_type{brickEntity.entity()});
    }

    void Game::placeBricks() const {
        constexpr int cols =3;
        constexpr int rows = 18;
        constexpr int top_margin = 20;
        constexpr int side_margin = 20;
        constexpr float spacing = 5.0f;

        constexpr float bw = BRICK_W * BRICKS_TEX_SCALE;
        constexpr float bh = BRICK_H * BRICKS_TEX_SCALE;

        // left side
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                SDL_FPoint pos = {
                    side_margin + static_cast<float>(c) * (bw + spacing),
                    top_margin + static_cast<float>(r) * (bh + spacing)
                };
                createBrick(pos, r+c);
            }
        }

        // right side
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                SDL_FPoint pos = {
                    WIN_WIDTH - side_margin - static_cast<float>(c) * (bw + spacing),
                    top_margin + static_cast<float>(r) * (bh + spacing)
                };
                createBrick(pos, r+2-c);
            }
        }
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

        bgTex = IMG_LoadTexture(ren, "res/bg.png");
        if (!bgTex) {
            std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
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

    void Game::prepareWalls() const {
        /* ---------- 1. Body & shape templates ---------- */
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody; // every wall is static

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1; // irrelevant for static bodies
        shapeDef.isSensor = false; // ← hard surface
        shapeDef.enableSensorEvents = false;

        /* ---------- Helper to spawn a wall entity with Goal ---------- */
        auto makeGoalWall = [&](float cx, float cy, float hx, float hy, bool isLeft) {
            b2Polygon box = b2MakeBox(hx, hy);
            bodyDef.position = {cx, cy};
            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);
            // Create Bagel entity with Goal
            Entity wallEntity = Entity::create();
            wallEntity.addAll(
                Transform{{cx * BOX_SCALE, cy * BOX_SCALE}, 0},
                Collider{body},
                Goal{ isLeft , !isLeft}
            );
            b2Body_SetUserData(body, new ent_type{wallEntity.entity()});
            cout.flush();
            cout << "Created wall at (" << cx << ", " << cy << ") with Goal: "
                 << (isLeft ? "Left" : "Right") << " entity number is: " << wallEntity.entity().id << std::endl;

        };

        /* ---------- 2. Small helper to spawn one wall (Box2D only) ---------- */
        auto makeWall = [&](float cx, float cy, float hx, float hy) {
            b2Polygon box = b2MakeBox(hx, hy);
            bodyDef.position = {cx, cy};
            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);
        };

        /* ---------- 3. Dimensions in physics-metres ---------- */
        constexpr float W = WIN_WIDTH / BOX_SCALE; // play-field width
        constexpr float H = WIN_HEIGHT / BOX_SCALE; // play-field height
        constexpr float T = 1.0f; // wall half-thickness

        /* ---------- 4. Build the four walls ---------- */
        // ⬆ Top
        makeWall(W * 0.5f, -T, W * 0.5f, T);

        // ⬇ Bottom
        makeWall(W * 0.5f, H + T, W * 0.5f, T);

        // ⬅ Left (entity with Goal, isLeft = true)
        makeGoalWall(-T, H * 0.5f, T, H * 0.5f, true);

        // ➡ Right (entity with Goal, isLeft = false)
        makeGoalWall(W + T, H * 0.5f, T, H * 0.5f, false);
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

    void Game::input_system() const {
        static const Mask mask = MaskBuilder()
                .set<Keys>()
                .set<Intent>()
                .build();

        SDL_PumpEvents();
        const bool *keys = SDL_GetKeyboardState(nullptr);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                const auto &k = World::getComponent<Keys>(e);
                auto &i = World::getComponent<Intent>(e);

                i.up = keys[k.up];
                i.down = keys[k.down];
                i.tilt_up = keys[k.tilt_up];
                i.tilt_down = keys[k.tilt_down];
            }
        }
    }

    void Game::move_system() const {
        static const Mask mask = MaskBuilder()
                .set<Intent>()
                .set<Collider>()
                .build();
        //todo : block the paddels from going through the floor and ceiling
        // also adding the tilting on tilt_up / tilt_down
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                const auto &i = World::getComponent<Intent>(e);
                const auto &c = World::getComponent<Collider>(e);

                const float f = i.up ? -5 : i.down ? 5 : 0;
                b2Body_SetLinearVelocity(c.b, {0, f});
            }
        }
    }

    void Game::draw_system() const {
        static const Mask mask = MaskBuilder()
                .set<Transform>()
                .set<Drawable>()
                .build();

        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, bgTex, nullptr, nullptr);


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

    void Game::collision_detector_system () const {
        static const Mask mask = MaskBuilder()
                .set<Collider>()
                .build();
        const b2ContactEvents& events = b2World_GetContactEvents(boxWorld);
        for (int i = 0; i < events.beginCount; ++i) {
            std::cout << "Collision detected between: " << std::endl;
            b2BodyId e1 = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
            b2BodyId e2 = b2Shape_GetBody(events.beginEvents[i].shapeIdA);

            auto *visitor1 = static_cast<ent_type*>(b2Body_GetUserData(e1));
            cout << "Entity 1: " << (visitor1 ? std::to_string(visitor1->id) : "null") << std::endl;
            auto *visitor2 = static_cast<ent_type*>(b2Body_GetUserData(e2));
            cout << "Entity 2: " << (visitor2 ? std::to_string(visitor2->id) : "null") << std::endl;
            if (visitor1 && World::mask(*visitor1).test(mask))
                World::addComponent(*visitor1, IsCollision{});
            if (visitor2 && World::mask(*visitor2).test(mask)) {
                visitor2 = static_cast<ent_type*>(b2Body_GetUserData(e2));
                World::addComponent(*visitor2, IsCollision{});
            }
        }
    }

    void Game::brick_system () const {
        static const Mask mask = MaskBuilder()
            .set<Breakable>()
            .set<IsCollision>()
            .set<ChangePart>()
            .set<Drawable>()
            .set<Collider>()
            .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                auto &c = World::getComponent<ChangePart>(e);
                auto &d = World::getComponent<Drawable>(e);

                c.coords.idx++;
                if (c.coords.idx >= NUM_BRICK_STATE) {
                    // destroy the brick
                    b2BodyId body = World::getComponent<Collider>(e).b;
                    if (b2Body_IsValid(body)) {
                        b2DestroyBody(body);
                        World::destroyEntity(e);
                    }

                }

                else {
                    // change the part of the brick
                    World::delComponent<IsCollision>(e);
                    World::getComponent<Drawable>(e) = {
                        c.coords.pos[c.coords.idx],
                        {d.size.x, d.size.y}
                    };
                }
            }
        }
    }
    void Game::score_system () const {
        static const Mask mask = MaskBuilder()
            .set<IsCollision>()
            .set<Goal>()
            .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                auto &winner = World::getComponent<Goal>(e);
                if (winner.left) {
                    std::cout << "Left player scored!" << std::endl;
                } else {
                    std::cout << "Right player scored!" << std::endl;
                }

            }
        }
    }




    Game::Game() {
        if (!prepareWindowAndTexture())
            return;
        SDL_srand(time(nullptr));

        prepareBoxWorld();
        prepareWalls();
        createBall();
        createPads();
        placeBricks();
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

    void Game::run() const {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        auto start = SDL_GetTicks();
        bool quit = false;

        //todo : ask moshe about this class since i dont really understand it
        // as you ca see there is this class InputSystem and there is a system called input_system()

        // InputSystem is;
        while (!quit) {
            //is.updateEntities();
            //first updateEntities() for all systems

            //is.update();
            //then update() for all systems

            World::step();
            //finally World::step() to clear added() array

            input_system();
            move_system();

            box_system();
            collision_detector_system();
            brick_system();
            score_system();
            // todo: implement reset on all bricks lost (maybe?)
            draw_system();

            const auto end = SDL_GetTicks();
            if (const auto elapsed = end - start) {
                SDL_Delay(static_cast<Uint32>(GAME_FRAME - static_cast<float>(elapsed)));
            }
            start += static_cast<Uint64>(GAME_FRAME);

            // todo : move this in to the input system
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
