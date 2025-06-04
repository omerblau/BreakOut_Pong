#pragma once
#include "../include/game.h"

using namespace bagel;

 namespace game {

     Game::Game() {
         prepareWindow();
         prepareBoxWorld();
         createWalls();
         createBall();

     }


     Game::~Game()
     {
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


     void Game::run()
     {
         SDL_SetRenderDrawColor(ren, 0,0,0,255);
         auto start = SDL_GetTicks();
         bool quit = false;

         while (!quit) {


             //World::step();
             //finally World::step() to clear added() array

             //input_system();
             //move_system();
             box_system();
             draw_system();

             auto end = SDL_GetTicks();
             if (end-start < GAME_FRAME) {
                 SDL_Delay(GAME_FRAME - (end-start));
             }
             start += GAME_FRAME;

             SDL_Event e;
             while (SDL_PollEvent(&e)) {
                 if (e.type == SDL_EVENT_QUIT)
                     quit = true;
                 else if ((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE))
                     quit = true;
             }
         }
     }


     void Game::box_system() const
     {
         static const Mask mask = MaskBuilder()
             .set<Collider>()
             .set<Transform>()
             .build();
         static constexpr float	BOX2D_STEP = 1.f/FPS;

         b2World_Step(boxWorld, BOX2D_STEP, 4);

         for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
             if (World::mask(e).test(mask)) {
                 b2Transform t = b2Body_GetTransform(World::getComponent<Collider>(e).body);
                 World::getComponent<Transform>(e) = {
                     {t.p.x*BOX_SCALE, t.p.y*BOX_SCALE},
                     RAD_TO_DEG * b2Rot_GetAngle(t.q)
                 };
             }
         }
     }

     bool Game::prepareWindow()
     {
         if (!SDL_Init(SDL_INIT_VIDEO)) {
             std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
             return false;
         }

         if (!SDL_CreateWindowAndRenderer("Breakout Pong", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
             std::cerr << "SDL_CreateWindowAndRenderer Error: " << SDL_GetError() << "\n";
             return false;
         }

         tex = IMG_LoadTexture(ren, "res/spritesheet.png");
         if (!tex) {
             std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
             return false;
         }
         return true;
     }

         // // Load background texture
         // bgTex = IMG_LoadTexture(ren, "res/cat_bg.png");
         // if (!bgTex) {
         //     std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
         //     return false;
         // }



     void Game::prepareBoxWorld()
     {
         b2WorldDef worldDef = b2DefaultWorldDef();
         worldDef.gravity = {0.0f, 18.0f};
         boxWorld = b2CreateWorld(&worldDef);



     }

     void Game::createBall() const
     {
         b2BodyDef ballBodyDef = b2DefaultBodyDef();
         ballBodyDef.type = b2_dynamicBody;
         ballBodyDef.fixedRotation = false;
         ballBodyDef.position = {WIN_WIDTH/2/BOX_SCALE, WIN_HEIGHT/2/BOX_SCALE};

         b2ShapeDef ballShapeDef = b2DefaultShapeDef();
         ballShapeDef.enableSensorEvents = true;
         ballShapeDef.density = 1;
         ballShapeDef.material.friction = 0;
         ballShapeDef.material.restitution = 1.1f;
         b2Circle ballCircle = {0,0,BALL_TEX.w*BALL_TEX_SCALE/BOX_SCALE/2};

         b2BodyId ballBody = b2CreateBody(boxWorld, &ballBodyDef);
         b2CreateCircleShape(ballBody, &ballShapeDef, &ballCircle);

         float xs = SDL_randf()/2+.25f;
         if (SDL_rand(2))
             xs = -xs;
         float ys = SDL_sqrtf(1-xs*xs);
         if (SDL_rand(2))
             ys = -ys;
         b2Body_SetLinearVelocity(ballBody, {xs*30,ys*30});

         Entity ballEntity = Entity::create();
         ballEntity.addAll(
             Transform{{},0},
             Drawable{{BALL_TEX}, {BALL_TEX.w*BALL_TEX_SCALE, BALL_TEX.h*BALL_TEX_SCALE}, Game::tex},
             Collider{ballBody}
         );
         b2Body_SetUserData(ballBody, new ent_type{ballEntity.entity()});
     }



     // void Game::createPad(const SDL_FRect& r, const SDL_FPoint& p, const Keys& k) const
     // {
     //     b2BodyDef padBodyDef = b2DefaultBodyDef();
     //     padBodyDef.type = b2_kinematicBody;
     //     padBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
     //     b2BodyId padBody = b2CreateBody(boxWorld, &padBodyDef);
     //
     //     b2ShapeDef padShapeDef = b2DefaultShapeDef();
     //     padShapeDef.density = 1;
     //
     //     b2Polygon padBox = b2MakeBox(r.w*PAD_TEX_SCALE/BOX_SCALE/2, r.h*PAD_TEX_SCALE/BOX_SCALE/2);
     //     b2CreatePolygonShape(padBody, &padShapeDef, &padBox);
     //
     //     Entity::create().addAll(
     //         Transform{{},0},
     //         Drawable{r, {r.w*PAD_TEX_SCALE, r.h*PAD_TEX_SCALE}},
     //         Collider{padBody},
     //         Intent{},
     //         k
     //     );
     // }


     void Game::createWalls()
     {
         b2BodyDef bodyDef = b2DefaultBodyDef();
         bodyDef.type = b2_staticBody;

         b2ShapeDef shapeDef = b2DefaultShapeDef();
         shapeDef.density = 1.0f;

         b2Polygon box = b2MakeBox(WIN_WIDTH / 2.0f / BOX_SCALE, 10.0f / BOX_SCALE);

         bodyDef.position = {WIN_WIDTH / 2.0f / BOX_SCALE, WIN_HEIGHT / BOX_SCALE - 0.4f};
         b2BodyId ground = b2CreateBody(boxWorld, &bodyDef);
         b2CreatePolygonShape(ground, &shapeDef, &box);

         box = b2MakeBox(5, WIN_HEIGHT/2.0f/BOX_SCALE);

         bodyDef.position = {-5.5f ,WIN_HEIGHT/2.0f/BOX_SCALE};
         b2BodyId left = b2CreateBody(boxWorld, &bodyDef);
         b2CreatePolygonShape(left, &shapeDef, &box);

         bodyDef.position = {WIN_WIDTH/BOX_SCALE + 4.5f, WIN_HEIGHT/2.0f/BOX_SCALE};
         b2BodyId right = b2CreateBody(boxWorld, &bodyDef);
         b2CreatePolygonShape(right, &shapeDef, &box);
     }


     void Game::move_system() const
     {
         static const Mask mask = MaskBuilder()
             .set<Intent>()
             .set<Collider>()
             .build();
     }


     void Game::draw_system() const
     {
         static const Mask mask = MaskBuilder()
             .set<Transform>()
             .set<Drawable>()
             .build();

         SDL_RenderClear(ren);

         for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
             if (World::mask(e).test(mask)) {
                 const auto& d = World::getComponent<Drawable>(e);
                 const auto& t = World::getComponent<Transform>(e);

                 const SDL_FRect dst = {
                     t.p.x-d.size.x/2,
                     t.p.y-d.size.y/2,
                     d.size.x, d.size.y};

                 SDL_RenderTextureRotated(
                     ren, tex, &d.part, &dst, t.angle,
                     nullptr, SDL_FLIP_NONE);
             }
         }

         SDL_RenderPresent(ren);
     }










 }