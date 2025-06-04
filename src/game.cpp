#pragma once
#include "../include/game.h"

 namespace game {
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

         // // Load background texture
         // bgTex = IMG_LoadTexture(ren, "res/cat_bg.png");
         // if (!bgTex) {
         //     std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << "\n";
         //     return false;
         // }


         return true;
     }

     void Game::prepareBoxWorld()
     {
         b2WorldDef worldDef = b2DefaultWorldDef();
         worldDef.gravity = {0.0f, 18.0f};
         boxWorld = b2CreateWorld(&worldDef);
     }

     void Pong::createBall() const
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
             Drawable{{BALL_TEX}, {BALL_TEX.w*BALL_TEX_SCALE, BALL_TEX.h*BALL_TEX_SCALE}},
             Collider{ballBody}
         );
         b2Body_SetUserData(ballBody, new ent_type{ballEntity.entity()});
     }
     void Pong::createPad(const SDL_FRect& r, const SDL_FPoint& p, const Keys& k) const
     {
         b2BodyDef padBodyDef = b2DefaultBodyDef();
         padBodyDef.type = b2_kinematicBody;
         padBodyDef.position = {p.x / BOX_SCALE, p.y / BOX_SCALE};
         b2BodyId padBody = b2CreateBody(boxWorld, &padBodyDef);

         b2ShapeDef padShapeDef = b2DefaultShapeDef();
         padShapeDef.density = 1;

         b2Polygon padBox = b2MakeBox(r.w*PAD_TEX_SCALE/BOX_SCALE/2, r.h*PAD_TEX_SCALE/BOX_SCALE/2);
         b2CreatePolygonShape(padBody, &padShapeDef, &padBox);

         Entity::create().addAll(
             Transform{{},0},
             Drawable{r, {r.w*PAD_TEX_SCALE, r.h*PAD_TEX_SCALE}},
             Collider{padBody},
             Intent{},
             k
         );
     }

     bool Pong::prepareWindowAndTexture()
     {
         if (!SDL_Init(SDL_INIT_VIDEO)) {
             cout << SDL_GetError() << endl;
             return false;
         }

         if (!SDL_CreateWindowAndRenderer(
             "Pong", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
             cout << SDL_GetError() << endl;
             return false;
             }
         SDL_Surface *surf = IMG_Load("res/pong.png");
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



 }