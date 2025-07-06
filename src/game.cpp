#include "../include/components.h"
#include "../include/game.h"
#include <iostream>
#include <array>
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include "../lib/box2d/src/body.h"
#include "../include/bagel.h"


using namespace std;
using namespace bagel;

namespace game {

    void Game::addSideTag(const ent_type e, const bool isRight) {
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
            static_cast<float>(WIN_WIDTH) / 2 / BOX_SCALE,
            static_cast<float>(WIN_HEIGHT) / 2 / BOX_SCALE
        };

        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.enableSensorEvents = true;
        ballShapeDef.enableContactEvents = true;
        ballShapeDef.density = 1;
        ballShapeDef.material.friction = 0;
        ballShapeDef.material.restitution = 1.0f;
        constexpr b2Circle ballCircle = {
            0, 0,
            BALL_COORDS.w * BALL_TEX_SCALE / BOX_SCALE / 2
        };

        const b2BodyId ballBody = b2CreateBody(boxWorld, &ballBodyDef);
        b2CreateCircleShape(ballBody, &ballShapeDef, &ballCircle);

        float xs = SDL_randf() / 2 + .25f;
        if (SDL_rand(2)) xs = -xs;
        float ys = SDL_sqrtf(1 - xs * xs);
        if (SDL_rand(2)) ys = -ys;
        b2Body_SetLinearVelocity(ballBody, {xs * BALL_INIT_MPS, ys * BALL_INIT_MPS});

        const Entity ballEntity = Entity::create();
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

    void Game::createPad(const SDL_FRect &r, const SDL_FPoint &p, const Keys &k, const bool isRight) const {
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
        const Entity padEntity = Entity::create(); // may be const
        padEntity.addAll(
            Transform{{}, 0},
            Drawable{r, {r.w * PAD_TEX_SCALE, r.h * PAD_TEX_SCALE}},
            Collider{padBody},
            Intent{},
            k
        );
        addSideTag(padEntity.entity(), isRight);

        if (!isRight && players == Players::Single)
            padEntity.add(AI{});
        b2Body_SetUserData(padBody, new ent_type{padEntity.entity()});
    }

    void Game::createPads() const {
        if (players == Players::Single)
            createPad(PAD_COORDS, {PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, {}, false);
        else
            createPad(PAD_COORDS, {PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, LEFT_KEYS, false);

        createPad(PAD_COORDS, {WIN_WIDTH - PAD_Y_MARGIN, static_cast<int>(WIN_HEIGHT / 2)}, RIGHT_KEYS, true);
    }

    void Game::createBrick(const SDL_FPoint &pos, const int row, const bool isRight) const {
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_staticBody;
        def.position = {pos.x / BOX_SCALE, pos.y / BOX_SCALE};

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1;

        const b2Polygon box = b2MakeBox(
            BRICK_W * BRICKS_TEX_SCALE / BOX_SCALE / 2,
            BRICK_H * BRICKS_TEX_SCALE / BOX_SCALE / 2
        );

        const b2BodyId body = b2CreateBody(boxWorld, &def);
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

        const Entity brickEntity = Entity::create();
        brickEntity.addAll(
            Transform{pos, 0},
            Drawable{coords.pos[0], {BRICK_W * BRICKS_TEX_SCALE, BRICK_H * BRICKS_TEX_SCALE}},
            ChangePart{coords},
            Collider{body},
            Breakable{}
        );

        addSideTag(brickEntity.entity(), isRight);

        b2Body_SetUserData(body, new ent_type{brickEntity.entity()});
    }

    void Game::placeBricks() const {
        constexpr int cols = BRICK_COLS;
        constexpr int rows = BRICK_ROWS;
        constexpr int top_margin = 36;
        constexpr int side_margin = 20;
        constexpr float spacing = 3.8f;

        constexpr float bw = BRICK_W * BRICKS_TEX_SCALE;
        constexpr float bh = BRICK_H * BRICKS_TEX_SCALE;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                SDL_FPoint pos = {
                    side_margin + static_cast<float>(c) * (bw + spacing),
                    top_margin + static_cast<float>(r) * (bh + spacing)
                };
                createBrick(pos, r + c, false);
            }
        }

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
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1;
        shapeDef.isSensor = false;
        shapeDef.enableSensorEvents = false;

        auto makeGoalWall = [&](const float cx, const float cy, const float hx, const float hy, const bool isLeft) {
            const b2Polygon box = b2MakeBox(hx, hy);
            bodyDef.position = {cx, cy};
            const b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);

            const Entity wallEntity = Entity::create();
            wallEntity.addAll(
                Transform{{cx * BOX_SCALE, cy * BOX_SCALE}, 0},
                Collider{body},
                Goal{isLeft, !isLeft}
            );
            b2Body_SetUserData(body, new ent_type{wallEntity.entity()});
        };

        auto makeWall = [&](const float cx, const float cy, const float hx, const float hy) {
            const b2Polygon box = b2MakeBox(hx, hy);
            bodyDef.position = {cx, cy};
            const b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);
        };

        constexpr float W = WIN_WIDTH / BOX_SCALE;
        constexpr float H = WIN_HEIGHT / BOX_SCALE;
        constexpr float T = 1.0f;

        makeWall(W * 0.5f, -T, W * 0.5f, T);

        makeWall(W * 0.5f, H + T, W * 0.5f, T);

        makeGoalWall(-T, H * 0.5f, T, H * 0.5f, true);

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
                const b2Transform t = b2Body_GetTransform(World::getComponent<Collider>(e).body);
                World::getComponent<Transform>(e) = {
                    {t.p.x * BOX_SCALE, t.p.y * BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };
            }
        }
    }

    void Game::constraints_system() {
        paddle_bounds();
        ball_speed_cap();
    }

    void Game::input_system() {
        static const Mask mask = MaskBuilder()
                .set<Keys>()
                .set<Intent>()
                .build();

        static const Mask aiMask = MaskBuilder()
                .set<AI>()
                .build();

        SDL_PumpEvents();
        const bool *keys = SDL_GetKeyboardState(nullptr);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(aiMask)) continue;
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

    void Game::ai_input_system() const {
        if (players != Players::Single) return;

        static const Mask ballMask = MaskBuilder()
            .set<Ball>()
            .set<Transform>()
            .set<Collider>()
            .build();

        static const Mask aiMask = MaskBuilder()
            .set<Transform>()
            .set<Intent>()
            .set<AI>()
            .build();

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

            if (ai.cooldownFrames > 0) {
                --ai.cooldownFrames;
                return;
            }

            const float paddleX = t.p.x;

            if (ballVel.x > 0 && ai.targetY != -1.0f) {
                ai.targetY = -1.0f;
            }

            if (ballVel.x < 0 && ai.targetY == -1.0f) {
                float x = ballPos.x;
                float y = ballPos.y;
                const float vx = ballVel.x;
                float vy = ballVel.y;
                float finalVy = vy;

                while (x > paddleX) {
                    static constexpr float top = 0.0f;
                    static constexpr auto bottom = static_cast<float>(WIN_HEIGHT);
                    float timeToWall = (vy > 0) ? (bottom - y) / vy : (top - y) / vy;
                    float timeToPaddle = (paddleX - x) / vx;

                    const float dt = std::min(timeToWall, timeToPaddle);

                    x += vx * dt;
                    y += vy * dt;

                    if (y <= top || y >= bottom) {
                        vy = -vy;
                        y = std::clamp(y, top, bottom);
                    }

                    finalVy = vy;

                    if (x <= paddleX) break;
                }

                ai.targetY = y + static_cast<float>(SDL_rand(201) - 100);
                ai.tiltDirection = (finalVy > 0) ? 1 : -1;
                ai.tiltFramesRemaining = SDL_rand(10); // random 0–5
            }

            i.up = i.down = i.tilt_up = i.tilt_down = false;

            if (ai.targetY != -1.0f) {
                static constexpr float tolerance = 10.0f;
                if (ai.targetY < t.p.y - tolerance) {
                    i.up = true;
                } else if (ai.targetY > t.p.y + tolerance) {
                    i.down = true;
                } else {
                    if (ai.tiltFramesRemaining > 0) {
                        if (ai.tiltDirection > 0) i.tilt_up = true;
                        else i.tilt_down = true;
                        ai.tiltFramesRemaining--;
                    }
                }
            }
        }
    }

    void Game::move_system() {
        static const Mask mask = MaskBuilder()
                .set<Intent>()
                .set<Collider>()
                .set<Drawable>() // NEW
                .build();

        static constexpr float DT = 1.f / FPS; // one physics step
        static constexpr float WORLD_H = WIN_HEIGHT / BOX_SCALE;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                const auto &i = World::getComponent<Intent>(e);
                const auto &c = World::getComponent<Collider>(e);
                const auto &dr = World::getComponent<Drawable>(e); // NEW
                float vy = i.up ? -PAD_MOVE : i.down ? PAD_MOVE : 0.f;

                if (vy != 0.f) {
                    const b2Transform xf = b2Body_GetTransform(c.body);
                    const float y = xf.p.y;

                    /* half-extents from sprite size → metres */
                    float halfW = 0.5f * dr.size.x / BOX_SCALE;
                    float halfH = 0.5f * dr.size.y / BOX_SCALE;

                    /* project half-height along world-Y */
                    const float halfY = std::max(halfH, halfW) - VIS_MARGIN_M;

                    const float yNext = y + vy * DT;

                    if ((vy < 0.f && yNext - halfY < 0.f) || (vy > 0.f && yNext + halfY > WORLD_H))
                        vy = 0.f; // cancel velocity
                }

                b2Body_SetLinearVelocity(c.body, {0.f, vy});

                const float angVel = (i.tilt_up ? -PAD_TILT : (i.tilt_down ? PAD_TILT : 0.f)) * DEG_TO_RAD;
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

    void Game::createPowerUp(const SDL_FRect &r,const SDL_FPoint& pos, const PUKind kind) const {
        b2BodyDef bd = b2DefaultBodyDef();
        bd.type          = b2_dynamicBody;
        bd.position  = { pos.x / BOX_SCALE, pos.y / BOX_SCALE };
        const b2BodyId b = b2CreateBody(boxWorld, &bd);

        b2ShapeDef sd = b2DefaultShapeDef();
        sd.isSensor   = true;
        sd.enableSensorEvents = true;
        const b2Polygon box = b2MakeBox(20 / BOX_SCALE, 20 / BOX_SCALE);
        b2CreatePolygonShape(b, &sd, &box);

        const float vx = (pos.x > static_cast<float>(WIN_WIDTH) / 2) ? -PU_SPEED_PPS : PU_SPEED_PPS;
        const bool side = (pos.x <= static_cast<float>(WIN_WIDTH) / 2);

        const Entity e = Entity::create();
        e.addAll(
            Transform{ pos, 0 },
            Drawable{ r, { 40, 40 } },   // use heart sprite
            Collider{ b },
            EnlargePU{},
            Falling{vx,side}
        );

        addSideTag(e.entity(), side);
        b2Body_SetUserData(b, new ent_type{e.entity()});

        switch(kind){
            case PUKind::EnlargeSelf: e.add(PU_EnlargeSelf{});  break;
            case PUKind::ShrinkEnemy: e.add(PU_ShrinkEnemy{});  break;
            case PUKind::ExtraBall:   e.add(PU_ExtraBall{});    break;
            case PUKind::Coin:        e.add(PU_Coin{});        break;
        }

        b2Body_SetLinearVelocity(b, {vx, 0.0f});
    }


    void Game::collision_detector_system() const {
        const b2ContactEvents& ce = b2World_GetContactEvents(boxWorld);
        for (int i = 0; i < ce.beginCount; ++i) {
            handleCollisionPair(ce.beginEvents[i].shapeIdA, ce.beginEvents[i].shapeIdB);
        }

        const b2SensorEvents& se = b2World_GetSensorEvents(boxWorld);
        for (int i = 0; i < se.beginCount; ++i)
            handleCollisionPair(se.beginEvents[i].sensorShapeId, se.beginEvents[i].visitorShapeId);
    }

    void Game::handleCollisionPair(b2ShapeId sa, b2ShapeId sb) const {
        if (!b2Shape_IsValid(sa) || !b2Shape_IsValid(sb)) return;
        const b2BodyId ba = b2Shape_GetBody(sa);
        const b2BodyId bb = b2Shape_GetBody(sb);
        const auto *ea = static_cast<ent_type *>(b2Body_GetUserData(ba));
        const auto *eb = static_cast<ent_type *>(b2Body_GetUserData(bb));
        if (!ea || !eb) return;

        auto is = [&](auto comp) {
            return [&](const ent_type e) {
                return World::mask(e).test(Component<decltype(comp)>::Bit);
            };
        };

        auto isBrick = is(Breakable{});
        auto isBall = is(Ball{});
        auto isPowerUp = is(Falling{});
        auto isPaddle = is(Intent{});
        auto isTimer = is(PUtimer{});
        auto isGoal = is(Goal{});

        if ((isBrick(*ea) && isBall(*eb)) || (isBrick(*eb) && isBall(*ea))) {
            const ent_type brick = isBrick(*ea) ? *ea : *eb;
            const ent_type ball = isBall(*ea) ? *ea : *eb;

            const bool protectL = World::mask(brick).test(Component<ProtectLeft >::Bit);
            const bool protectR = World::mask(brick).test(Component<ProtectRight>::Bit);

            if (protectL || protectR)
            {
                const bool ballLeft  = World::mask(ball).test(Component<leftBallTouchedLast >::Bit);
                const bool ballRight = World::mask(ball).test(Component<rightBallTouchedLast>::Bit);

                const bool allowGhost =
                      (protectL && ballLeft) ||
                      (protectR && ballRight);

                if (allowGhost)
                    return;
            }

            World::addComponent(brick, IsCollision{});
            if (isTimer(ball)) {
                auto &timer = World::getComponent<PUtimer>(ball);
                timer.hitsLeft--;
            }
        }

        if ((isGoal(*ea) && isBall(*eb)) || (isGoal(*eb) && isBall(*ea))) {
            const ent_type goal = isGoal(*ea) ? *ea : *eb;
            World::addComponent(goal, IsCollision{});
        }

        if ((isPaddle(*ea) && isPowerUp(*eb)) || (isPaddle(*eb) && isPowerUp(*ea))) {
            const ent_type pad = isPaddle(*ea) ? *ea : *eb;
            const ent_type pu = isPowerUp(*ea) ? *ea : *eb;

            const bool padIsLeft = World::mask(pad).test(Component<TagLeft>::Bit);
            const bool puIsLeft = World::mask(pu).test(Component<TagLeft>::Bit);

            if (padIsLeft != puIsLeft) return;

            applyPowerUp(pad, pu);
        }

        if ((isPaddle(*ea) && isBall(*eb)) || (isPaddle(*eb) && isBall(*ea))) {
            const ent_type pad = isPaddle(*ea) ? *ea : *eb;
            const ent_type ball = isBall(*ea)   ? *ea : *eb;

            const bool padIsLeft = World::mask(pad).test(Component<TagLeft>::Bit);

            World::delComponent<leftBallTouchedLast >(ball);
            World::delComponent<rightBallTouchedLast>(ball);

            if (padIsLeft)
                World::addComponent(ball, leftBallTouchedLast{});
            else
                World::addComponent(ball, rightBallTouchedLast{});

            updateProtectBricks(padIsLeft);

            if (isTimer(pad)) {
                auto &timer = World::getComponent<PUtimer>(pad);
                timer.hitsLeft--;
            }
        }
    }

    void Game::applyPowerUp(ent_type pad, ent_type pu) const {
        if (World::mask(pu).test(Component<PU_EnlargeSelf>::Bit)) {
            enlargePaddle(pad);
        } else if (World::mask(pu).test(Component<PU_ShrinkEnemy>::Bit)) {
            const ent_type opp = findOpponentOf(pad);
            shrinkPadel(opp);
        } else if (World::mask(pu).test(Component<PU_ExtraBall>::Bit)) {
            const bool isRight = World::mask(pad).test(Component<TagRight>::Bit);
            spawnExtraBallAt(getPaddlePosition(pad), isRight);
        } else if (World::mask(pu).test(Component<PU_Coin>::Bit)) {
            const bool isRight = World::mask(pad).test(Component<TagRight>::Bit);
            createProtectBricks(isRight);
        }

        const b2BodyId body = World::getComponent<Collider>(pu).body;
        DestroyBodySafe(body);
        World::destroyEntity(pu);
    }

    void Game::createProtectBricks(const bool protectRight) const
    {

        static constexpr float startY = 400.0f;
        static constexpr int   bricks = 5;
        static constexpr float bw     = BRICK_W * BRICKS_TEX_SCALE;
        static constexpr float bh     = BRICK_H * BRICKS_TEX_SCALE;
        static constexpr float gap    = 5.0f;

        replaceProtectBricks(protectRight);

        const float brickX = protectRight
        ? WIN_WIDTH - PAD_Y_MARGIN - WALL_GAP
        : PAD_Y_MARGIN + WALL_GAP;

        for (int i = 0; i < bricks; ++i)
        {
            const SDL_FPoint pos = { brickX, startY + (bh + gap) * static_cast<float>(i)};

            b2BodyDef def = b2DefaultBodyDef();
            def.type      = b2_dynamicBody;
            def.linearDamping   = 4.0f;
            def.position  = { pos.x / BOX_SCALE, pos.y / BOX_SCALE };
            const b2BodyId body = b2CreateBody(boxWorld, &def);

            b2ShapeDef sd = b2DefaultShapeDef();
            sd.density   = 200.0f;
            sd.isSensor   = true;
            b2Polygon box = b2MakeBox(bw * 0.5f / BOX_SCALE,
                                      bh * 0.5f / BOX_SCALE);
            b2CreatePolygonShape(body, &sd, &box);

            Entity e = Entity::create();
            e.addAll(
                Transform{ pos, 0 },
                Drawable{ PROTECT_PASSIVE, { bw, bh } }, // special sprite
                Collider{ body },
                TTL{ FPS * 20 }
            );

            if (protectRight)
                e.add(ProtectRight{});
            else
                e.add(ProtectLeft{});
            b2Body_SetUserData(body, new ent_type{e.entity()});
        }
    }


    void Game::updateProtectBricks(bool leftActive) const
    {
        static const Mask leftMask  = MaskBuilder()
        .set<ProtectLeft>().set<Collider>().set<Drawable>().build();

        static const Mask rightMask = MaskBuilder()
            .set<ProtectRight>().set<Collider>().set<Drawable>().build();

        auto rebuild = [&](ent_type e, bool makeSensor)
        {
            const b2BodyId oldB = World::getComponent<Collider>(e).body;
            const b2Transform tf = b2Body_GetTransform(oldB);
            DestroyBodySafe(oldB);

            b2BodyDef def = b2DefaultBodyDef();
            def.type      = b2_dynamicBody;
            def.position  = tf.p;
            def.rotation =  tf.q;
            const b2BodyId newB = b2CreateBody(boxWorld, &def);

            b2ShapeDef sd = b2DefaultShapeDef();
            sd.isSensor   = makeSensor;
            const auto& dr = World::getComponent<Drawable>(e);
            const float hx = 0.5f * dr.size.x / BOX_SCALE;
            const float hy = 0.5f * dr.size.y / BOX_SCALE;
            const b2Polygon box = b2MakeBox(hx, hy);
            b2CreatePolygonShape(newB, &sd, &box);

            World::getComponent<Collider>(e).body = newB;
            b2Body_SetUserData(newB, new ent_type{ e.id });

            World::getComponent<Drawable>(e).part =
                makeSensor ? PROTECT_PASSIVE : PROTECT_ACTIVE;
        };

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id)
        {
            if (World::mask(e).test(leftMask))
                rebuild(e, leftActive);
            else if (World::mask(e).test(rightMask))
                rebuild(e, !leftActive);
        }
    }

    void Game::replaceProtectBricks(const bool isRightSide)
    {
        using namespace bagel;
        static const Mask leftMask  = MaskBuilder().set<ProtectLeft>().set<Collider>().build();
        static const Mask rightMask = MaskBuilder().set<ProtectRight>().set<Collider>().build();
        const Mask m = isRightSide ? rightMask : leftMask;
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id)
        {
            if (!World::mask(e).test(m)) continue;
            const b2BodyId b = World::getComponent<Collider>(e).body;
            DestroyBodySafe(b);
            World::destroyEntity(e);
        }
    }

    void Game::protect_brick_damp_system() {
        static const Mask mLeft  = MaskBuilder().set<ProtectLeft >().set<Collider>().build();
        static const Mask mRight = MaskBuilder().set<ProtectRight>().set<Collider>().build();
        static constexpr float DAMP = 0.01f;
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id)
            if (World::mask(e).test(mLeft) || World::mask(e).test(mRight))
            {
                const b2BodyId b = World::getComponent<Collider>(e).body;
                b2Vec2 vel = b2Body_GetLinearVelocity(b);
                vel.x *= DAMP;
                vel.y *= DAMP;
                b2Body_SetLinearVelocity(b, vel);
            }
    }

    void Game::ttl_system()
    {
        static const Mask m = MaskBuilder()
            .set<TTL>()
            .set<Collider>()   // need this to remove Box2D body
            .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id)
            if (World::mask(e).test(m))
            {
                auto &ttl = World::getComponent<TTL>(e);
                if (--ttl.framesLeft > 0) continue;

                const b2BodyId b = World::getComponent<Collider>(e).body;
                DestroyBodySafe(b);
                World::destroyEntity(e);
            }
    }


    void Game::spawnExtraBallAt(const SDL_FPoint &padPos, const bool isRight) const {
        static constexpr float MAX_DEVIATION = 0.3f;
        static constexpr float BUFFER = 2.0f;
        static constexpr float R          = BALL_COORDS.w * BALL_TEX_SCALE * 0.5f;
        static constexpr float HALF_PAD_W = PAD_COORDS.w  * PAD_TEX_SCALE * 0.5f;

        SDL_FPoint  spawnPos = padPos;
        spawnPos.x += isRight
                      ? -(R + HALF_PAD_W + BUFFER)
                      :  +(R + HALF_PAD_W + BUFFER);
        const float baseAngle = isRight ? M_PI : 0.0f;

        for (int i = 0; i < 2; ++i) {
            b2BodyDef bd = b2DefaultBodyDef();
            bd.type     = b2_dynamicBody;
            bd.position = { spawnPos.x / BOX_SCALE, spawnPos.y / BOX_SCALE };
            const b2BodyId body = b2CreateBody(boxWorld, &bd);

            b2ShapeDef sd = b2DefaultShapeDef();
            sd.enableSensorEvents  = true;
            sd.enableContactEvents = true;
            sd.density             = 1;
            sd.material.friction    = 0;
            sd.material.restitution = 1.0f;

            b2Circle circ{ 0, 0, R / BOX_SCALE };
            b2CreateCircleShape(body, &sd, &circ);
            const float delta  = (SDL_randf()*2.f - 1.f) * MAX_DEVIATION;
            const float angle  = baseAngle + delta;
            const float vx = std::cos(angle) * BALL_INIT_MPS;
            const float vy = std::sin(angle) * BALL_INIT_MPS;
            b2Body_SetLinearVelocity(body, { vx, vy });

            Entity e = Entity::create();
            e.addAll(
                Transform{ spawnPos, 0 },
                Drawable{ BALL_COORDS,
                          { BALL_COORDS.w * BALL_TEX_SCALE,
                            BALL_COORDS.h * BALL_TEX_SCALE } },
                Collider{ body },
                Ball{},
                PUtimer{1}
            );
            b2Body_SetUserData(body, new ent_type{ e.entity() });
        }
    }
    SDL_FPoint Game::getPaddlePosition(const ent_type pad) {
        return World::getComponent<Transform>(pad).p;
    }

    ent_type Game::findOpponentOf(const ent_type pad) {
        const bool padIsLeft = World::mask(pad).test(Component<TagLeft>::Bit);
        static const Mask leftMask  = MaskBuilder().set<Intent>().set<Collider>().set<TagLeft>().build();
        static const Mask rightMask = MaskBuilder().set<Intent>().set<Collider>().set<TagRight>().build();
        const Mask &target = padIsLeft ? rightMask : leftMask;
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(target)) {
                return e;
            }
        }
        return ent_type{};
    }

    void Game::enlargePaddle(const ent_type pad) const {
        const b2BodyId oldBody = World::getComponent<Collider>(pad).body;
        const b2Transform tf = b2Body_GetTransform(oldBody);
        const b2Vec2 pos = tf.p;
        const float angle = b2Rot_GetAngle(tf.q);
        DestroyBodySafe(oldBody);
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        const b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});

        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        static constexpr float hx = PAD_LONG_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        static constexpr float hy = PAD_LONG_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);
        World::getComponent<Collider>(pad).body = newBody;
        b2Body_SetUserData(newBody, new ent_type{pad});
        auto& dr = World::getComponent<Drawable>(pad);
        dr.part = PAD_LONG_COORDS;
        dr.size = { PAD_LONG_COORDS.w * PAD_TEX_SCALE,
                    PAD_LONG_COORDS.h * PAD_TEX_SCALE };

        World::addComponent(pad, PUtimer{3});
    }

    void Game::shrinkPadel(const ent_type pad) const {
        const b2BodyId oldBody = World::getComponent<Collider>(pad).body;
        const b2Transform tf = b2Body_GetTransform(oldBody);
        const b2Vec2 pos = tf.p;
        const float angle = b2Rot_GetAngle(tf.q);

        DestroyBodySafe(oldBody);
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        const b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});

        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        static constexpr float hx = PAD_SHORT_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        static constexpr float hy = PAD_SHORT_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);

        World::getComponent<Collider>(pad).body = newBody;
        b2Body_SetUserData(newBody, new ent_type{pad});

        auto &dr = World::getComponent<Drawable>(pad);
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
                    const b2BodyId body = World::getComponent<Collider>(e).body;
                    if (b2Body_IsValid(body)) {
                        b2DestroyBody(body);
                        World::destroyEntity(e);
                    }
                } else {
                    World::delComponent<IsCollision>(e);
                    World::getComponent<Drawable>(e) = {
                        c.coords.pos[c.coords.idx],
                        {d.size.x, d.size.y}
                    };

                    if (++bricksBroken % HITS_NUM_PU_CREATION == 0)
                        createPowerUpRotating({ World::getComponent<Transform>(e).p.x, World::getComponent<Transform>(e).p.y });
                }
            }
        }
    }

    void Game::createPowerUpRotating(const SDL_FPoint &pos) const {
        static constexpr std::array<PUKind, 4> kinds = {
            PUKind::EnlargeSelf,
            PUKind::ShrinkEnemy,
            PUKind::Coin,
            PUKind::ExtraBall
        };
        static size_t idx = 0;
        const PUKind kind = kinds[idx];
        const SDL_FRect &sprite = spriteFor(kind);
        createPowerUp(sprite, pos, kind);
        idx = (idx + 1) % kinds.size();
    }

    void Game::powerup_move_system() {
        static const Mask m = MaskBuilder()
                .set<Falling>()
                .set<Transform>()
                .set<Collider>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(m)) {
                auto &t = World::getComponent<Transform>(e);
                const auto &c = World::getComponent<Collider>(e);
                const b2Transform tf = b2Body_GetTransform(c.body);
                t.p.x = tf.p.x * BOX_SCALE;
                t.p.y = tf.p.y * BOX_SCALE;
                if (t.p.x < -50 || t.p.x > WIN_WIDTH + 50) {
                    const b2BodyId b = c.body;
                    DestroyBodySafe(b);
                    World::destroyEntity(e);
                }
            }
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

            const auto &timer = World::getComponent<PUtimer>(e);

            if (timer.hitsLeft > 0)
                continue;
            if (World::mask(e).test(Component<Intent>::Bit)) {
                paddleBackToOG(e);
            } else if (World::mask(e).test(Component<Ball>::Bit)) {
                const b2BodyId body = World::getComponent<Collider>(e).body;
                DestroyBodySafe(body);
                World::destroyEntity(e);
            }
            World::delComponent<PUtimer>(e);
        }
    }

    void Game::paddleBackToOG(ent_type pad) const {
        const b2BodyId oldBody = World::getComponent<Collider>(pad).body;
        const b2Transform tf = b2Body_GetTransform(oldBody);
        const b2Vec2 pos = tf.p;
        const float angle = b2Rot_GetAngle(tf.q);
        DestroyBodySafe(oldBody);
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_kinematicBody;
        def.position = pos;
        const b2BodyId newBody = b2CreateBody(boxWorld, &def);
        b2Body_SetTransform(newBody, pos, b2Rot{std::cos(angle), std::sin(angle)});
        b2ShapeDef sd = b2DefaultShapeDef();
        sd.density = 0;
        sd.enableSensorEvents = true;

        static constexpr float hx = PAD_COORDS.w * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        static constexpr float hy = PAD_COORDS.h * PAD_TEX_SCALE / BOX_SCALE / 2.0f;
        const b2Polygon box = b2MakeBox(hx, hy);
        b2CreatePolygonShape(newBody, &sd, &box);

        World::getComponent<Collider>(pad).body = newBody;
        b2Body_SetUserData(newBody, new ent_type{pad});
        auto &dr = World::getComponent<Drawable>(pad);
        dr.part = PAD_COORDS;
        dr.size = {
            PAD_COORDS.w * PAD_TEX_SCALE,
            PAD_COORDS.h * PAD_TEX_SCALE
        };
    }
    void Game::score_system() const {
        static const Mask goalMask = MaskBuilder()
                .set<IsCollision>()
                .set<Goal>()
                .build();

        static const Mask leftBrickMask = MaskBuilder()
                .set<Transform>()
                .set<Breakable>()
                .set<ChangePart>()
                .set<Drawable>()
                .set<Collider>()
                .set<TagLeft>()
                .build();

        static const Mask rightBrickMask = MaskBuilder()
                .set<Transform>()
                .set<Breakable>()
                .set<ChangePart>()
                .set<Drawable>()
                .set<Collider>()
                .set<TagRight>()
                .build();

        if (mode == GameMode::FirstGoal) {
            for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
                if (World::mask(e).test(goalMask)) {
                    auto &winner = World::getComponent<Goal>(e);
                    if (winner.left) {
                        gameState = GameState::RIGHT_WIN;
                    } else {
                        gameState = GameState::LEFT_WIN;
                    }
                }
            }
        } else {
            bool isLeft = false;
            bool isRight = false;
            for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
                if (World::mask(e).test(leftBrickMask))
                    isLeft = true;
                if (World::mask(e).test(rightBrickMask))
                    isRight = true;
            }
            if (!isLeft) {
                gameState = GameState::RIGHT_WIN;
                return;
            }
            if (!isRight)
                gameState = GameState::LEFT_WIN;
        }
    }
    void Game::cleanup_collision_system() {
        static const Mask mask = MaskBuilder()
                .set<IsCollision>()
                .build();
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask)) {
                World::delComponent<IsCollision>(e);
            }
        }
    }

    void Game::paddle_bounds() {
        static const Mask paddleMask = MaskBuilder()
                .set<Collider>()
                .set<Intent>()
                .set<Drawable>()
                .build();

        constexpr float WORLD_H = WIN_HEIGHT / BOX_SCALE;
        constexpr float BASE = 90.0f * DEG_TO_RAD;
        constexpr float MAX_OFF = 45.0f * DEG_TO_RAD;
        constexpr float MIN_TILT = BASE - MAX_OFF;
        constexpr float MAX_TILT = BASE + MAX_OFF;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(paddleMask))
                continue;

            const b2BodyId body = World::getComponent<Collider>(e).body;
            const b2Transform xf = b2Body_GetTransform(body);
            b2Vec2 pos = xf.p;
            float ang = b2Rot_GetAngle(xf.q);
            const auto &d = World::getComponent<Drawable>(e);
            const float halfW = 0.5f * d.size.x / BOX_SCALE; // metres
            const float halfH = 0.5f * d.size.y / BOX_SCALE;

            const float halfY = std::fabs(std::cos(ang)) * halfH +
                                std::fabs(std::sin(ang)) * halfW -
                                VIS_MARGIN_M;
            bool clamped = false;
            if (pos.y - halfY < 0.0f) {
                pos.y = halfY;
                clamped = true;
            }
            if (pos.y + halfY > WORLD_H) {
                pos.y = WORLD_H - halfY;
                clamped = true;
            }
            if (clamped) {
                b2Body_SetTransform(body, pos, xf.q); // snap back
                b2Body_SetLinearVelocity(body, {0.0f, 0.0f}); // stop push
            }
            const float fixed = std::clamp(ang, MIN_TILT, MAX_TILT);
            if (fixed != ang) {
                const b2Rot rq{std::cos(fixed), std::sin(fixed)};
                b2Body_SetTransform(body, pos, rq);
                b2Body_SetAngularVelocity(body, 0.0f);
            }
        }
    }

    void Game::ball_speed_cap() {
        static const Mask colliderMask = MaskBuilder()
                .set<Collider>()
                .build();

        static constexpr float MAX_V2 = BALL_MAX_MPS * BALL_MAX_MPS * SPEED_MULTIPLIER;
        static constexpr float MIN_V2 = BALL_MIN_MPS * BALL_MIN_MPS * SPEED_MULTIPLIER;

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(colliderMask)) continue;
            if (World::mask(e).test(Component<Intent>::Bit)) continue; // skip paddles

            const b2BodyId b = World::getComponent<Collider>(e).body;
            if (b2Body_GetType(b) != b2_dynamicBody) continue; // bricks / walls

            auto [vx, vy] = b2Body_GetLinearVelocity(b);
            const float v2 = vx * vx + vy * vy;
            if (v2 > MAX_V2) {
                const float scale = BALL_MAX_MPS / SDL_sqrtf(v2);
                b2Body_SetLinearVelocity(b, {vx * scale, vy * scale});
            } else if (v2 < MIN_V2 && v2 > 0.f) {
                const float scale = BALL_MIN_MPS / SDL_sqrtf(v2);
                b2Body_SetLinearVelocity(b, {vx * scale, vy * scale});
            }
        }
    }

    void Game::destroy_all_entities() {
        static const Mask colliderMask = MaskBuilder()
                .set<Collider>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(colliderMask)) {
                const b2BodyId body = World::getComponent<Collider>(e).body;
                b2DestroyBody(body);
            }
        }
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            World::destroyEntity(e);
        }
    }

    void Game::DestroyBodySafe(const b2BodyId b) {
        if (b2Body_IsValid(b))
            b2DestroyBody(b);
    }

    void Game::reset_game() {
        destroy_all_entities();
    }

    void Game::create_game() const {
        prepareWalls();
        createBall();
        createPads();
        placeBricks();
    }
}
