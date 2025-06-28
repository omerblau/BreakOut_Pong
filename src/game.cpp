// This is game.cpp file

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
    /// !!! the reason these below are not split in to different files is as follows
    /// the bagel.h engine uses a macro to give id's to entities and bit's location for mask based on instances of a template class instance
    /// these start from 0 for each new file.
    /// ─── Factory functions ──────────────────────────────────────────


    void Game::addSideTag(ent_type e, bool isRight) {
        if (isRight)
            World::addComponent(e, TagRight{});
        else
            World::addComponent(e, TagLeft{});
    }


    void Game::createBall() const {
        b2BodyDef ballBodyDef = b2DefaultBodyDef();
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.isBullet = true;
        ballBodyDef.fixedRotation = false;
        ballBodyDef.position = {
            WIN_WIDTH / 2 / BOX_SCALE,
            WIN_HEIGHT / 2 / BOX_SCALE
        };

        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.enableSensorEvents = true;
        ballShapeDef.enableContactEvents = true;
        ballShapeDef.density = 1;
        ballShapeDef.material.friction = 0;
        ballShapeDef.material.restitution = 1.0f;
        b2Circle ballCircle = {
            0, 0,
            BALL_COORDS.w * BALL_TEX_SCALE / BOX_SCALE / 2
        };

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
            Drawable{
                BALL_COORDS,
                {
                    BALL_COORDS.w * BALL_TEX_SCALE,
                    BALL_COORDS.h * BALL_TEX_SCALE
                }
            },
            Collider{ballBody},
            Ball{}
        );
        b2Body_SetUserData(ballBody, new ent_type{ballEntity.entity()});
    }

    void Game::createPad(const SDL_FRect &r, const SDL_FPoint &p, const Keys &k, const PaddleSide side, bool IsRight) const {
        b2BodyDef padBodyDef = b2DefaultBodyDef();
        padBodyDef.type = b2_kinematicBody;
        padBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
        const b2BodyId padBody = b2CreateBody(boxWorld, &padBodyDef);

        constexpr float angleRad = 90.0f * DEG_TO_RAD;
        const b2Rot rot = {std::cos(angleRad), std::sin(angleRad)};
        b2Body_SetTransform(padBody, padBodyDef.position, rot);

        b2ShapeDef padShapeDef = b2DefaultShapeDef();
        padShapeDef.enableSensorEvents = true;
        padShapeDef.density = 0;

        const b2Polygon padBox = b2MakeBox(
            r.w * PAD_TEX_SCALE / BOX_SCALE / 2,
            r.h * PAD_TEX_SCALE / BOX_SCALE / 2);
        b2CreatePolygonShape(padBody, &padShapeDef, &padBox);
        Entity padEntity = Entity::create(); // may be const
        padEntity.addAll(
            Transform{{}, 0},
            Drawable{r, {r.w * PAD_TEX_SCALE, r.h * PAD_TEX_SCALE}},
            Collider{padBody},
            Intent{},
            k
        );
        // Add the tag separately
        addSideTag(padEntity.entity(), IsRight);

        if (side == PaddleSide::Left && players == Players::Single)
            padEntity.add(AI{});
        b2Body_SetUserData(padBody, new ent_type{padEntity.entity()});
    }

    void Game::createPads() const {
        if (players == Players::Single)
            createPad(PAD_COORDS, {PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, {}, PaddleSide::Left, false);
        else
            createPad(PAD_COORDS, {PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, LEFT_KEYS, PaddleSide::Left, false);

        createPad(PAD_COORDS, {WIN_WIDTH - PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, RIGHT_KEYS, PaddleSide::Right, true);
    }



    void Game::createBrick(const SDL_FPoint &pos, int row, bool isRight) const {
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

        // Add the tag separately
        addSideTag(brickEntity.entity(), isRight);

        b2Body_SetUserData(body, new ent_type{brickEntity.entity()});
    }

    void Game::placeBricks() const {
        constexpr int cols = 3;
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
                createBrick(pos, r + c, false);
            }
        }

        // right side
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                SDL_FPoint pos = {
                    WIN_WIDTH - side_margin - static_cast<float>(c) * (bw + spacing),
                    top_margin + static_cast<float>(r) * (bh + spacing)
                };
                createBrick(pos, r + 2 - c, true);
            }
        }
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
                Goal{isLeft, !isLeft}
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

    /// ─── ECS systems ───────────────────────────────────────────
    void Game::box_system() const {
        static const Mask mask = MaskBuilder()
                .set<Collider>()
                .set<Transform>()
                .build();
        static constexpr float BOX2D_STEP = 1.f / FPS;

        b2World_Step(boxWorld, BOX2D_STEP, 4);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                b2Transform t = b2Body_GetTransform(World::getComponent<Collider>(e).body);
                World::getComponent<Transform>(e) = {
                    {t.p.x * BOX_SCALE, t.p.y * BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
            }
        }
    }

    void Game::constraints_system() const {
        paddle_bounds();
        ball_speed_cap();
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

    // void Game::ai_input_system() const {
    //     if (players != Players::Single) return;
    //
    //     Mask ballMask = MaskBuilder()
    //         .set<Ball>()
    //         .set<Transform>()
    //         .build();
    //
    //     Mask aiPaddleMask = MaskBuilder()
    //         .set<Transform>()
    //         .set<Intent>()
    //         .set<AI>()
    //         .build();
    //
    //     // Get ball's position
    //     SDL_FPoint ballPos{};
    //     for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
    //         if (!World::mask(e).test(ballMask)) continue;
    //         const auto &t = World::getComponent<Transform>(e);
    //         ballPos = t.p;
    //         break;
    //     }
    //
    //     if (ballPos.x > WIN_WIDTH / 2) return;
    //
    //     // For each AI-controlled paddle, update its Intent
    //     for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
    //         if (!World::mask(e).test(aiPaddleMask)) continue;
    //         const auto &t = World::getComponent<Transform>(e);
    //         auto &i = World::getComponent<Intent>(e);
    //         i.up = i.down = false;
    //
    //         const float tolerance = 10.0f; // deadzone
    //         if (ballPos.y < t.p.y - tolerance) {
    //             i.up = true;
    //         } else if (ballPos.y > t.p.y + tolerance) {
    //             i.down = true;
    //         }
    //
    //         // Optional: Add randomness for human-like error
    //         // if (rand() % 100 < 5) i.up = i.down = false;
    //     }
    // }

    void Game::ai_input_system() const {
        if (players != Players::Single) return;

        Mask ballMask = MaskBuilder()
            .set<Ball>()
            .set<Transform>()
            .set<Collider>()
            .build();

        Mask aiMask = MaskBuilder()
            .set<Transform>()
            .set<Intent>()
            .set<AI>()
            .build();

        // Ball state
        SDL_FPoint ballPos{};
        b2Vec2 ballVel{};

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(ballMask)) continue;
            const auto &t = World::getComponent<Transform>(e);
            const auto &c = World::getComponent<Collider>(e);

            ballPos = t.p;
            ballVel = b2Body_GetLinearVelocity(c.body);
            break;
        }

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(aiMask)) continue;

            const auto &t = World::getComponent<Transform>(e);
            auto &i = World::getComponent<Intent>(e);
            auto &ai = World::getComponent<AI>(e);

            float paddleX = t.p.x;

            // Reset AI target if ball moves away
            if (ballVel.x > 0 && ai.targetY != -1.0f) {
                ai.targetY = -1.0f;
            }

            // Calculate intercept Y if ball is coming toward paddle and no target set
            if (ballVel.x < 0 && ai.targetY == -1.0f) {
                float x = ballPos.x;
                float y = ballPos.y;
                float vx = ballVel.x;
                float vy = ballVel.y;

                const float top = 0.0f;
                const float bottom = static_cast<float>(WIN_HEIGHT);

                while (x > paddleX) {
                    float timeToWall = (vy > 0) ? (bottom - y) / vy : (top - y) / vy;
                    float timeToPaddle = (paddleX - x) / vx;

                    float dt = std::min(timeToWall, timeToPaddle);

                    x += vx * dt;
                    y += vy * dt;

                    if (y <= top || y >= bottom) {
                        vy = -vy;
                        y = std::clamp(y, top, bottom);
                    }

                    if (x <= paddleX) break;
                }

                ai.targetY = y;
            }

            // Move paddle toward target
            i.up = i.down = false;
            if (ai.targetY != -1.0f) {
                const float tolerance = 10.0f;
                if (ai.targetY < t.p.y - tolerance) {
                    i.up = true;
                } else if (ai.targetY > t.p.y + tolerance) {
                    i.down = true;
                }
            }
        }
    }

    void Game::move_system() const {
        static const Mask mask = MaskBuilder()
                .set<Intent>()
                .set<Collider>()
                .build();
        //todo : block the paddles from going through the floor and ceiling
        // also adding the tilting on tilt_up / tilt_down

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                const auto &i = World::getComponent<Intent>(e);
                const auto &c = World::getComponent<Collider>(e);

                const float f = i.up ? -PAD_MOVE : i.down ? PAD_MOVE : 0.f;
                b2Body_SetLinearVelocity(c.body, {0, f});

                /* smooth tilting – keeps rotating while key is held */
                float angVel = (i.tilt_up ? -PAD_TILT : (i.tilt_down ? PAD_TILT : 0.f)) * DEG_TO_RAD;
                b2Body_SetAngularVelocity(c.body, angVel);
            }
        }
    }

    void Game::draw_system() const {
        static const Mask mask = MaskBuilder()
                .set<Transform>()
                .set<Drawable>()
                .build();

        SDL_RenderClear(ren);

        // Draw game elements only if playing
        if (gameState == GameState::PLAYING || gameState == GameState::PAUSED) {
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
                        ren, tex, &d.part, &dst, t.angle,
                        nullptr, SDL_FLIP_NONE);
                }
            }
        }

        // Draw overlay screens
        switch (gameState) {
            case GameState::PAUSED:
                SDL_RenderTexture(ren, pauseTex, nullptr, nullptr);
                break;
            case GameState::LEFT_WIN:
                SDL_RenderTexture(ren, leftWinTex, nullptr, nullptr);
                break;
            case GameState::RIGHT_WIN:
                SDL_RenderTexture(ren, rightWinTex, nullptr, nullptr);
                break;
            default:
                break;
        }

        SDL_RenderPresent(ren);
    }

    /*void Game::collision_detector_system() const {
        static const Mask mask = MaskBuilder()
                .set<Collider>()
                .build();
        const b2ContactEvents &events = b2World_GetContactEvents(boxWorld);
        for (int i = 0; i < events.beginCount; ++i) {
            std::cout << "Collision detected between: " << std::endl;
            b2BodyId e1 = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
            b2BodyId e2 = b2Shape_GetBody(events.beginEvents[i].shapeIdA);

            void* userData1 = b2Body_GetUserData(e1);
            void* userData2 = b2Body_GetUserData(e2);

            if (userData1) {
                ent_type entity1{static_cast<id_type>(reinterpret_cast<uintptr_t>(userData1))};
                if (entity1.id <= World::maxId().id && World::mask(entity1).test(mask)) {
                    World::addComponent(entity1, IsCollision{});
                }
            }

            if (userData2) {
                ent_type entity2{static_cast<id_type>(reinterpret_cast<uintptr_t>(userData2))};
                if (entity2.id <= World::maxId().id && World::mask(entity2).test(mask)) {
                    World::addComponent(entity2, IsCollision{});
                }
            }
        }
    }*/


    void Game::createPowerUp(const SDL_FRect &r,const SDL_FPoint& pos, PUKind kind) const {
        // 1. physics (sensor)
        b2BodyDef bd = b2DefaultBodyDef();
        bd.type          = b2_dynamicBody;
        bd.position  = { pos.x / BOX_SCALE, pos.y / BOX_SCALE };
        b2BodyId b = b2CreateBody(boxWorld, &bd);

        b2ShapeDef sd = b2DefaultShapeDef();
        sd.isSensor   = true;               // collect only, no collision response
        sd.enableSensorEvents = true;
        b2Polygon box = b2MakeBox(20 / BOX_SCALE, 20 / BOX_SCALE);
        b2CreatePolygonShape(b, &sd, &box);

        /* -------- choose horizontal direction -------- */
        float vx = (pos.x > WIN_WIDTH / 2) ? -PU_SPEED_PPS :  PU_SPEED_PPS;
        bool side = (pos.x <= WIN_WIDTH / 2);

        // 2. entity
        Entity e = Entity::create();
        e.addAll(
            Transform{ pos, 0 },
            Drawable{ r, { 40, 40 } },   // use heart sprite
            Collider{ b },
            EnlargePU{},
            Falling{vx,side}
        );

        addSideTag(e.entity(), side); // add tag for left/right player to collect
        b2Body_SetUserData(b, new ent_type{e.entity()});

        switch(kind){
            case PUKind::EnlargeSelf: e.add(PU_EnlargeSelf{});  break;
            case PUKind::ShrinkEnemy: e.add(PU_ShrinkEnemy{});  break;
            case PUKind::ExtraBall:   e.add(PU_ExtraBall{});    break;
        }

        /* give the Box2D body the same velocity (m/s) */
        b2Body_SetLinearVelocity(b, { vx, 0.0f });
    }


    // void Game::collision_detector_system () const {
    //     static const Mask mask = MaskBuilder()
    //             .set<Collider>()
    //             .build();
    //
    //
    //     const b2ContactEvents& events = b2World_GetContactEvents(boxWorld);
    //     for (int i = 0; i < events.beginCount; ++i) {
    //         std::cout << "Collision detected between: " << std::endl;
    //         b2BodyId e1 = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
    //         b2BodyId e2 = b2Shape_GetBody(events.beginEvents[i].shapeIdA);
    //
    //         auto *visitor1 = static_cast<ent_type*>(b2Body_GetUserData(e1));
    //         cout << "Entity 1: " << (visitor1 ? std::to_string(visitor1->id) : "null") << std::endl;
    //         auto *visitor2 = static_cast<ent_type*>(b2Body_GetUserData(e2));
    //         cout << "Entity 2: " << (visitor2 ? std::to_string(visitor2->id) : "null") << std::endl;
    //         if (visitor1 && World::mask(*visitor1).test(mask))
    //             World::addComponent(*visitor1, IsCollision{});
    //         if (visitor2 && World::mask(*visitor2).test(mask)) {
    //             visitor2 = static_cast<ent_type*>(b2Body_GetUserData(e2));
    //             World::addComponent(*visitor2, IsCollision{});
    //         }
    //     }
    // }



    void Game::collision_detector_system() const
    {
        /* bitmasks for quick classifying */
        auto isBrick   =[&](ent_type e){return World::mask(e).test(Component<Breakable>::Bit);};
        auto isBall    =[&](ent_type e){return World::mask(e).test(Component<Ball>::Bit);};
        auto isPowerUp =[&](ent_type e){return World::mask(e).test(Component<Falling>::Bit);};
        auto isPaddle  =[&](ent_type e){return World::mask(e).test(Component<Intent>::Bit);};
        auto isTimer  =[&](ent_type e){return World::mask(e).test(Component<PUtimer>::Bit);};


        auto handlePair = [&](b2ShapeId sa, b2ShapeId sb)
        {
            b2BodyId ba = b2Shape_GetBody(sa);
            b2BodyId bb = b2Shape_GetBody(sb);
            auto* ea = static_cast<ent_type*>(b2Body_GetUserData(ba));
            auto* eb = static_cast<ent_type*>(b2Body_GetUserData(bb));
            if (!ea || !eb) return;

            /* ---------- Brick × Ball ---------- */

            if ((isBrick(*ea) && isBall(*eb)) || (isBrick(*eb) && isBall(*ea))) {
                ent_type brick = isBrick(*ea) ? *ea : *eb;
                ent_type ball  = isBall(*ea)  ? *ea : *eb;

                // brick for hitting
                World::addComponent(brick, IsCollision{});


                if (isTimer(ball)) {
                    auto& timer = World::getComponent<PUtimer>(ball);
                    timer.hitsLeft--;
                    std::cout << "Ball was hit! Remaining hits: " << timer.hitsLeft << std::endl;
                }

            }

            /* ---------- Paddle × Power-Up ---------- */
            if ( (isPaddle(*ea) && isPowerUp(*eb)) ||
                 (isPaddle(*eb) && isPowerUp(*ea)) )
            {
                ent_type pad = isPaddle(*ea)  ? *ea : *eb;
                ent_type pu  = isPowerUp(*ea) ? *ea : *eb;

                // Only allow pickup if they match the same side
                const bool padIsLeft = World::mask(pad).test(Component<TagLeft>::Bit);
                const bool puIsLeft  = World::mask(pu).test(Component<TagLeft>::Bit);

                if (padIsLeft != puIsLeft)
                    return; // not the same side, ignore

                applyPowerUp(pad, pu);


            }


            /* ---------- Paddle × Ball ---------- */
            if ((isPaddle(*ea) && isBall(*eb)) ||
                (isPaddle(*eb) && isBall(*ea)))
            {
                ent_type pad = isPaddle(*ea) ? *ea : *eb;

                if (isTimer(pad)) {
                    auto& timer = World::getComponent<PUtimer>(pad);
                    timer.hitsLeft--;
                    std::cout << "Paddle was hit! Remaining hits: " << timer.hitsLeft << std::endl;
                }
            }

        };

        /* ① Contact-Events  */
        const b2ContactEvents& ce = b2World_GetContactEvents(boxWorld);
        for (int i = 0; i < ce.beginCount; ++i)
            handlePair(ce.beginEvents[i].shapeIdA,
                       ce.beginEvents[i].shapeIdB);


        /* --------------- 2. Sensor-Events ----------------- */
        const b2SensorEvents&  se = b2World_GetSensorEvents(boxWorld);
        for (int i = 0; i < se.beginCount; ++i) {
            handlePair(se.beginEvents[i].sensorShapeId,
                       se.beginEvents[i].visitorShapeId);
            cout << "matka and sensor" << std::endl;
        }


    }


    // In Game.cpp:

    void Game::applyPowerUp(ent_type pad, ent_type pu) const
    {
        // Determine which kind of power-up this is and apply its effect:
        if (World::mask(pu).test(Component<PU_EnlargeSelf>::Bit)) {
            // Enlarge self
            enlargePaddle(pad);
        }
        else if (World::mask(pu).test(Component<PU_ShrinkEnemy>::Bit)) {
            // find opponent
            ent_type opp = findOpponentOf(pad);
            // Shrink enemy paddle:
            shrinkPadel(opp);
        }
        else if (World::mask(pu).test(Component<PU_ExtraBall>::Bit)) {
            bool isRight = World::mask(pad).test(Component<TagRight>::Bit);
            // Spawn an extra ball for this player
            spawnExtraBallAt(getPaddlePosition(pad), isRight);
        }
        else if (World::mask(pu).test(Component<PU_Coin>::Bit)) {
            // e.g. increment score
            //addScore(pad, 1);
        }

        // Cleanup the power-up entity:
        b2BodyId body = World::getComponent<Collider>(pu).body;
        b2DestroyBody(body);
        World::destroyEntity(pu);
    }

    void Game::spawnExtraBallAt(const SDL_FPoint& padPos, bool isRight) const
{
    constexpr float MAX_DEVIATION = 0.3f;   // ‎±17° סטייה
    constexpr float BUFFER        = 2.0f;   // מרחק ביטחון קטן כדי לא לגעת במחבט

    /* ---- מחשבים נקודת ספאון מעט לפני המחבט ---- */
    const float R            = BALL_COORDS.w * BALL_TEX_SCALE * 0.5f;            // רדיוס הכדור-בפיקסלים
    const float HALF_PAD_W   = PAD_COORDS.w  * PAD_TEX_SCALE * 0.5f;             // חצי רוחב המחבט
    SDL_FPoint  spawnPos     = padPos;
    spawnPos.x += isRight
                  ? -(R + HALF_PAD_W + BUFFER)   // מחבט ימין → מזיזים שמאלה (-X)
                  :  +(R + HALF_PAD_W + BUFFER); // מחבט שמאל → מזיזים ימינה (+X)

    /* ---- כיוון “קדימה” בסיסי של הכדור ---- */
    const float baseAngle = isRight ? M_PI : 0.0f;   // ימין → π (שמאלה), שמאל → 0 (ימינה)

    for (int i = 0; i < 2; ++i) {

        /* ---------- 1. גוף Box2D ---------- */
        b2BodyDef bd = b2DefaultBodyDef();
        bd.type     = b2_dynamicBody;
        bd.position = { spawnPos.x / BOX_SCALE, spawnPos.y / BOX_SCALE };
        b2BodyId body = b2CreateBody(boxWorld, &bd);

        b2ShapeDef sd = b2DefaultShapeDef();
        sd.enableSensorEvents  = true;
        sd.enableContactEvents = true;
        sd.density             = 1;
        sd.material.friction    = 0;
        sd.material.restitution = 1.0f;

        b2Circle circ{ 0, 0, R / BOX_SCALE };
        b2CreateCircleShape(body, &sd, &circ);

        /* ---------- 2. מהירות התחלה ---------- */
        float delta  = (SDL_randf()*2.f - 1.f) * MAX_DEVIATION;
        float angle  = baseAngle + delta;

        float vx = std::cos(angle) * BALL_INIT_MPS;
        float vy = std::sin(angle) * BALL_INIT_MPS;
        b2Body_SetLinearVelocity(body, { vx, vy });

        /* ---------- 3. יצירת ישות Bagel ---------- */
        Entity e = Entity::create();
        e.addAll(
            Transform{ spawnPos, 0 },
            Drawable{ BALL_COORDS,
                      { BALL_COORDS.w * BALL_TEX_SCALE,
                        BALL_COORDS.h * BALL_TEX_SCALE } },
            Collider{ body },
            Ball{},
            PUtimer{1}                 // כדור זמני – יימחק כשפוגע בלבנה
        );
        b2Body_SetUserData(body, new ent_type{ e.entity() });
    }
}



    // get center position of a paddle
    SDL_FPoint Game::getPaddlePosition(ent_type pad) const {
        return World::getComponent<Transform>(pad).p;
    }

    ent_type Game::findOpponentOf(ent_type pad) const {
        const bool padIsLeft = World::mask(pad).test(Component<TagLeft>::Bit);

        // change mask according to the side of the paddle
        static const Mask leftMask  = MaskBuilder().set<Intent>().set<Collider>().set<TagLeft>().build();
        static const Mask rightMask = MaskBuilder().set<Intent>().set<Collider>().set<TagRight>().build();

        const Mask &target = padIsLeft ? rightMask : leftMask;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(target)) {
                return e;
            }
        }
        return ent_type{}; // if we didn't find an opponent, return an empty entity
    }



    void Game::enlargePaddle(ent_type pad) const
    {
        // Get old body
        b2BodyId oldBody = World::getComponent<Collider>(pad).body;

        // Save position and rotation
        b2Transform tf = b2Body_GetTransform(oldBody);
        b2Vec2 pos = tf.p;
        float angle = b2Rot_GetAngle(tf.q);

        // Destroy the old body
        b2DestroyBody(oldBody);

        // Create new, larger body
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});

        // Add new shape
        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        const float hx = PAD_LONG_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const float hy = PAD_LONG_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);

        // Update the collider component to point to the new body
        World::getComponent<Collider>(pad).body = newBody;

        // Set user data again
        b2Body_SetUserData(newBody, new ent_type{pad});

        // Update the sprite
        auto& dr = World::getComponent<Drawable>(pad);
        dr.part = PAD_LONG_COORDS;
        dr.size = { PAD_LONG_COORDS.w * PAD_TEX_SCALE,
                    PAD_LONG_COORDS.h * PAD_TEX_SCALE };

        World::addComponent(pad, PUtimer{3});
    }


    void Game::shrinkPadel(ent_type pad) const
    {
        // Get old body
        b2BodyId oldBody = World::getComponent<Collider>(pad).body;

        // Save position and rotation
        b2Transform tf = b2Body_GetTransform(oldBody);
        b2Vec2 pos = tf.p;
        float angle = b2Rot_GetAngle(tf.q);

        // Destroy the old body
        b2DestroyBody(oldBody);

        // Create new, larger body
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});

        // Add new shape
        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        const float hx = PAD_SHORT_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const float hy = PAD_SHORT_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);

        // Update the collider component to point to the new body
        World::getComponent<Collider>(pad).body = newBody;

        // Set user data again
        b2Body_SetUserData(newBody, new ent_type{pad});

        // Update the sprite
        auto& dr = World::getComponent<Drawable>(pad);
        dr.part = PAD_SHORT_COORDS;
        dr.size = { PAD_SHORT_COORDS.w * PAD_TEX_SCALE,
                    PAD_SHORT_COORDS.h * PAD_TEX_SCALE };

        World::addComponent(pad, PUtimer{3});
    }







    void Game::brick_system() const {
        static const Mask mask = MaskBuilder()
                .set<Breakable>()
                .set<IsCollision>()
                .set<ChangePart>()
                .set<Drawable>()
                .set<Collider>()
                .build();


        static int bricksBroken = 0;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                auto &c = World::getComponent<ChangePart>(e);
                auto &d = World::getComponent<Drawable>(e);

                c.coords.idx++;
                if (c.coords.idx >= NUM_BRICK_STATE) {
                    // destroy the brick
                    b2BodyId body = World::getComponent<Collider>(e).body;
                    if (b2Body_IsValid(body)) {
                        b2DestroyBody(body);
                        World::destroyEntity(e);
                    }
                } else {
                    // change the part of the brick
                    World::delComponent<IsCollision>(e);
                    World::getComponent<Drawable>(e) = {
                        c.coords.pos[c.coords.idx],
                        {d.size.x, d.size.y}
                    };

                    if (++bricksBroken % 3 == 0) {
                        std::cout << "brick cunt!" << bricksBroken << std::endl;
                        createPowerUpRotating({ World::getComponent<Transform>(e).p.x, World::getComponent<Transform>(e).p.y });   // use the coordinates of the brick
                    }

                }
            }
        }
    }


    constexpr const SDL_FRect& spriteFor(PUKind k)
    {
        for (auto& [kind, rect] : spriteByKind)
            if (kind == k) return rect;
        // fallback
        return POWERUP_ENLARGE;
    }

    // --- power-up creation helper ---

    void Game::createPowerUpRotating(const SDL_FPoint& pos) const
    {
        static constexpr std::array<PUKind, 4> kinds = {
            PUKind::EnlargeSelf,
            PUKind::ShrinkEnemy,
            PUKind::Coin,
            PUKind::ExtraBall
        };

        static size_t idx = 0;


        PUKind kind = kinds[idx];
        const SDL_FRect& sprite = spriteFor(kind);

        createPowerUp(sprite, pos, kind);

        idx = (idx + 1) % kinds.size();
    }






    void Game::powerup_move_system() {
        static const Mask m = MaskBuilder()
                .set<Falling>()
                .set<Transform>()
                .set<Collider>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id)
            if (World::mask(e).test(m))
            {
                auto& t = World::getComponent<Transform>(e);
                auto& f = World::getComponent<Falling>(e);
                auto& c = World::getComponent<Collider>(e);

                // Box2D already advances the body; we only sync Transform for rendering
                b2Transform tf = b2Body_GetTransform(c.body);
                t.p.x = tf.p.x * BOX_SCALE;
                t.p.y = tf.p.y * BOX_SCALE;

                // if off–screen → delete
                if (t.p.x < -50 || t.p.x > WIN_WIDTH + 50)
                    World::destroyEntity(e);
            }
    }







    void Game::pu_timer_system() const {
        static const Mask m = MaskBuilder()
                .set<PUtimer>()
                .set<Drawable>()
                .set<Collider>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(m)) continue;

            auto& timer = World::getComponent<PUtimer>(e);

            if (timer.hitsLeft > 0)
                continue;

            // What to do when the timer expires?
            if (World::mask(e).test(Component<Intent>::Bit)) {
                // It's a paddle - shrink it
                paddleBackToOG(e);
            } else if (World::mask(e).test(Component<Ball>::Bit)) {
                // It's a ball - remove it
                b2BodyId body = World::getComponent<Collider>(e).body;
                b2DestroyBody(body);
                World::destroyEntity(e);
            }

            // Remove the timer
            World::delComponent<PUtimer>(e);
        }
    }

    void Game::paddleBackToOG(ent_type pad) const {
        // Get old body
        b2BodyId oldBody = World::getComponent<Collider>(pad).body;

        // Save position and rotation
        b2Transform tf = b2Body_GetTransform(oldBody);
        b2Vec2 pos = tf.p;
        float angle = b2Rot_GetAngle(tf.q);

        // Destroy old body
        b2DestroyBody(oldBody);

        // Create new, smaller body
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});

        // Add small shape
        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        const float hx = PAD_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const float hy = PAD_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);

        // Update the collider component to point to the new body
        World::getComponent<Collider>(pad).body = newBody;

        // Set user data again
        b2Body_SetUserData(newBody, new ent_type{pad});

        // Update the sprite
        auto& dr = World::getComponent<Drawable>(pad);
        dr.part = PAD_COORDS;
        dr.size = {
            PAD_COORDS.w * PAD_TEX_SCALE,
            PAD_COORDS.h * PAD_TEX_SCALE
        };
    }






    void Game::score_system() {
        static const Mask goalMask = MaskBuilder()
                .set<IsCollision>()
                .set<Goal>()
                .build();

        static const Mask ballMask = MaskBuilder()
                .set<Ball>()
                .set<Collider>()
                .build();

        bool scored = false;
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(goalMask)) {
                auto &winner = World::getComponent<Goal>(e);
                if (winner.left) {
                    std::cout << "Right player scored!" << std::endl;
                    gameState = GameState::RIGHT_WIN;
                } else {
                    std::cout << "Left player scored!" << std::endl;
                    gameState = GameState::LEFT_WIN;
                }
                scored = true;
            }
        }

        // Don't respawn ball immediately when someone wins
    }

    void Game::cleanup_collision_system() const {
        static const Mask mask = MaskBuilder()
                .set<IsCollision>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                World::delComponent<IsCollision>(e);
            }
        }
    }

    /// ─── Constraint helpers ────────────────────────────────────────
    void Game::paddle_bounds() const {
        static const Mask paddleMask = MaskBuilder()
                .set<Collider>()
                .set<Intent>() // only paddles have Intent
                .build();

        /* ─ constants (shared across frames) ─ */
        constexpr float HALF_W_M = (PAD_COORDS.w * PAD_TEX_SCALE) / BOX_SCALE / 2.0f;
        constexpr float HALF_H_M = (PAD_COORDS.h * PAD_TEX_SCALE) / BOX_SCALE / 2.0f;
        constexpr float WORLD_H = WIN_HEIGHT / BOX_SCALE;

        constexpr float BASE = 90.0f * DEG_TO_RAD; // vertical
        constexpr float MAX_OFF = 45.0f * DEG_TO_RAD;
        constexpr float MIN_TILT = BASE - MAX_OFF; // 45°
        constexpr float MAX_TILT = BASE + MAX_OFF; // 135°

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(paddleMask)) continue;

            const b2BodyId b = World::getComponent<Collider>(e).body;
            auto [p, q] = b2Body_GetTransform(b);
            b2Vec2 pos = p;
            float ang = b2Rot_GetAngle(q);

            /* dynamic half-height in world Y */
            const float cy = std::cos(ang);
            const float sy = std::sin(ang);
            const float halfY = std::fabs(cy) * HALF_H_M + std::fabs(sy) * HALF_W_M;

            /* Y clamp */
            bool yHit = false;
            if (pos.y - halfY < 0.f) {
                pos.y = halfY;
                yHit = true;
            }
            if (pos.y + halfY > WORLD_H) {
                pos.y = WORLD_H - halfY;
                yHit = true;
            }
            if (yHit) {
                b2Body_SetTransform(b, pos, q);
                b2Body_SetLinearVelocity(b, {0.f, 0.f});
                p = pos;
            }

            /* angle clamp */
            if (const float fixed = std::clamp(ang, MIN_TILT, MAX_TILT); fixed != ang) {
                const b2Rot r{std::cos(fixed), std::sin(fixed)};
                b2Body_SetTransform(b, p, r);
                b2Body_SetAngularVelocity(b, 0.f);
            }
        }
    }

    void Game::ball_speed_cap() const {
        static const Mask colliderMask = MaskBuilder()
                .set<Collider>()
                .build();

        constexpr float MAX_V2 = BALL_MAX_MPS * BALL_MAX_MPS * SPEED_MULTIPLIER;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(colliderMask)) continue;
            if (World::mask(e).test(Component<Intent>::Bit)) continue; // skip paddles

            const b2BodyId b = World::getComponent<Collider>(e).body;
            if (b2Body_GetType(b) != b2_dynamicBody) continue; // bricks/walls

            auto [x, y] = b2Body_GetLinearVelocity(b);
            if (const float v2 = x * x + y * y; v2 > MAX_V2) {
                const float scale = BALL_MAX_MPS / SDL_sqrtf(v2);
                b2Body_SetLinearVelocity(b, {x * scale, y * scale});
            }
        }
    }

    /// ─── Entity-lifecycle helpers ──────────────────────────────────
    void Game::destroy_all_entities() {
        // Destroy all entities with colliders first
        static const Mask colliderMask = MaskBuilder()
                .set<Collider>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(colliderMask)) {
                b2BodyId body = World::getComponent<Collider>(e).body;
                if (b2Body_IsValid(body)) {
                    b2DestroyBody(body);
                }
            }
        }

        // Destroy all entities
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            World::destroyEntity(e);
        }
    }

    void Game::reset_game() {
        // Destroy all game entities
        destroy_all_entities();
    }

    void Game::create_game() const {
        prepareWalls();
        createBall();
        createPads();
        placeBricks();
    }

    Game::~Game() {
        for (const auto &i: uiTex)
            if (i)
                SDL_DestroyTexture(i);

        if (b2World_IsValid(boxWorld))
            b2DestroyWorld(boxWorld);
        if (tex != nullptr)
            SDL_DestroyTexture(tex);
        if (bgTex != nullptr)
            SDL_DestroyTexture(bgTex);
        if (pauseTex != nullptr)
            SDL_DestroyTexture(pauseTex);
        if (leftWinTex != nullptr)
            SDL_DestroyTexture(leftWinTex);
        if (rightWinTex != nullptr)
            SDL_DestroyTexture(rightWinTex);
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
        bool gameExitToMenu = false;
        while (!gameExitToMenu && !appQuit) {
            World::step();
            const_cast<Game*>(this)->handle_game_state_input();

            // Only run game systems when playing
            if (gameState == GameState::PLAYING) {
                input_system();
                move_system();
                powerup_move_system();
                box_system();
                constraints_system();
                collision_detector_system();
                pu_timer_system();
                brick_system();
                const_cast<Game*>(this)->score_system();
                cleanup_collision_system();
            }



            // todo: implement reset on all bricks lost (maybe?)
            draw_system();

            pace_frame();
            // pace_frame_test(start);
            // auto end = SDL_GetTicks();
            // if (end-start < GAME_FRAME) {
            //     SDL_Delay(GAME_FRAME - (end-start));
            // }
            // start += GAME_FRAME;

            windowClosedClicked();
            quit = shouldQuit;
        }
    }
}
