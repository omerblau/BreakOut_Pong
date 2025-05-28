#include "Element.h"

#include <iostream>
#include <limits>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm> // for std::clamp

using namespace std;

#include "bagel.h"
using namespace bagel;


namespace element {
    // helper to find the first entity matching a mask
    static ent_type findEntity(const Mask &mask) {
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(mask))
                return e;
        }
        return ent_type{-1};
    }

    constexpr float BULLET_SPEED = 600.f; // px/sec

    /// init helpers  // @formatter:off
    bool Element::prepareWindowAndTexture() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return false;
        }

        if (!SDL_CreateWindowAndRenderer(
            "Flash Element Tower Defence", WIN_WIDTH, WIN_HEIGHT, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return false;
        }
        SDL_Surface *surf = IMG_Load("res/atlas.png");
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



        SDL_Surface* ds = IMG_Load("res/digits.png");
        if (!ds) {
            std::cerr << "IMG_Load digits.png failed: " << SDL_GetError() << std::endl;
            return false;
        }
        digits = SDL_CreateTextureFromSurface(ren, ds);
        if (!digits) {
            std::cerr << "CreateTexture digits failed: " << SDL_GetError() << std::endl;
            return false;
        }
        SDL_DestroySurface(ds);


        SDL_Surface* ds2 = IMG_Load("res/HUD.png");
        if (!ds2) {
            std::cerr << "HUD.png failed: " << SDL_GetError() << std::endl;
            return false;
        }
        hud = SDL_CreateTextureFromSurface(ren, ds2);
        if (!hud) {
            std::cerr << "CreateTexture HUD failed: " << SDL_GetError() << std::endl;
            return false;
        }
        SDL_DestroySurface(ds2);

        return true;
    }
    void Element::createMap() const {
        constexpr auto w = MAP_TEX.w * TEX_SCALE;
        constexpr auto h = MAP_TEX.h * TEX_SCALE;
        constexpr auto cy = MAP_TEX_PAD_Y + h * 0.5f; // centre-y
        constexpr auto cx = MAP_TEX_PAD_X + w * 0.5f; // centre-x

        Entity mapEntity = Entity::create();
        mapEntity.addAll(
            Transform{{cx, cy}, 0.f},
            Drawable{MAP_TEX, {MAP_TEX.w * TEX_SCALE, MAP_TEX.h * TEX_SCALE}}
        );
    }
    void Element::createBuyArrow() const {
        Entity buyArrowEntity = Entity::create();
        buyArrowEntity.addAll(
            Transform{{800, 380}, 0.f}, // place at (cx, cy)
            Drawable{BUY_ARROW_TEX, {BUY_ARROW_TEX.w * TEX_SCALE, BUY_ARROW_TEX.h * TEX_SCALE}}, // sprite + size
            UIButton_Tag{},
            Arrow_Tag{} // tower‐type tag
        );
    }
    void Element::createBuyCannon() const {
        Entity buyCannonEntity = Entity::create();
        buyCannonEntity.addAll(
            Transform{{880, 380}, 0.f}, // place at (cx, cy)
            Drawable{BUY_CANNON_TEX, {BUY_CANNON_TEX.w * TEX_SCALE, BUY_CANNON_TEX.h * TEX_SCALE}}, // sprite + size
            UIButton_Tag{},
            Cannon_Tag{} // tower‐type tag
        );
    }
    void Element::createBuyAir() const {
        Entity buyAirEntity = Entity::create();
        buyAirEntity.addAll(
            Transform{{960, 380}, 0.f}, // place at (cx, cy)
            Drawable{BUY_AIR_TEX, {BUY_AIR_TEX.w * TEX_SCALE, BUY_AIR_TEX.h * TEX_SCALE}}, // sprite + size
            UIButton_Tag{},
            Air_Tag{});// tower‐type tag
    }
    void Element::createNextLevelButton() const {
        Entity nextLevelButtonEntity = Entity::create();
        nextLevelButtonEntity.addAll(
            Transform{{790, 694}, 0.f}, // place at (cx, cy)
            Drawable{UI_NEXT_LEVEL, {UI_NEXT_LEVEL.w * TEX_SCALE, UI_NEXT_LEVEL.h * TEX_SCALE}}, // sprite + size
            UIButton_Tag{},
            NextLevel_Tag{}
        );
    }
    void Element::createCoinIcon() const {
        // pick a spot in screen‐coords:
        constexpr float X = 1100.f, Y = 226.f;
        SDL_FRect src = FRECT(sprite_ui_coin);
        SDL_FPoint size = { src.w * TEX_SCALE * 1.2f, src.h * TEX_SCALE  * 1.2f};

        Entity e = Entity::create();
        e.addAll(
            Transform{{X, Y}, 0.f},
            Drawable{ src, size },
            CoinIcon_Tag{}
        );
    }
    void Element::createHealthIcon() const {
        // pick another spot:
        constexpr float X = 910.f, Y = 226.f;
        SDL_FRect src = FRECT(sprite_ui_hp);
        SDL_FPoint size = { src.w * TEX_SCALE * 2.f, src.h * TEX_SCALE* 2.f };

        Entity e = Entity::create();
        e.addAll(
            Transform{{X, Y}, 0.f},
            Drawable{ src, size },
            HealthIcon_Tag{}
        );
    }
    void Element::createHUD() const {
        // Health panel
        Entity e1 = Entity::create();
        e1.addAll(
          Transform{{750.f, 100.f}, 0.f},
          Drawable{UI_HEALTH_TEX, {UI_HEALTH_TEX.w * TEX_SCALE, UI_HEALTH_TEX.h * TEX_SCALE}},
          HUD_Tag{}
        );

        // Money panel
        Entity e2 = Entity::create();
        e2.addAll(
          Transform{{950.f, 100.f}, 0.f},
          Drawable{UI_MONEY_TEX, {UI_MONEY_TEX.w * TEX_SCALE, UI_MONEY_TEX.h * TEX_SCALE}},
          HUD_Tag{}
        );

        // Level panel
        Entity e3 = Entity::create();
        e3.addAll(
          Transform{{1150.f, 100.f}, 0.f},
          Drawable{UI_LEVEL_TEX, {UI_LEVEL_TEX.w * TEX_SCALE, UI_LEVEL_TEX.h * TEX_SCALE}},
          HUD_Tag{}
        );
    }
    void Element::createUI() const {
        createMap();
        createBuyArrow();
        createBuyCannon();
        createBuyAir();
        createNextLevelButton();
        createCoinIcon();
        createHealthIcon();
        // createHUD();
    }
    void Element::createHeaders() const {
        {
            // Health
            SDL_FRect dstH{ 750.f, 100.f,
                            UI_HEALTH_TEX.w * TEX_SCALE,
                            UI_HEALTH_TEX.h * TEX_SCALE };
            SDL_RenderTexture(ren, hud, &UI_HEALTH_TEX, &dstH);

            // Money
            SDL_FRect dstM{ 950.f, 100.f,
                            UI_MONEY_TEX.w * TEX_SCALE,
                            UI_MONEY_TEX.h * TEX_SCALE };
            SDL_RenderTexture(ren, hud, &UI_MONEY_TEX, &dstM);

            // Level
            SDL_FRect dstL{1150.f, 100.f,
                           UI_LEVEL_TEX.w * TEX_SCALE,
                           UI_LEVEL_TEX.h * TEX_SCALE};
            SDL_RenderTexture(ren, hud, &UI_LEVEL_TEX, &dstL);
        }
    }

    void Element::createMouse() const {
        Entity mouseEntity = Entity::create();
        mouseEntity.addAll(
        Transform{{0,0}, 0.f},
        Drawable{{},{}},
        MouseInput{ 0, 0, false },
        Mouse_Tag{}
        );
    }
    void Element::createPlayer() const {
        Entity playerEntity = Entity::create();
        playerEntity.addAll(
            HP{20, 20},
            Gold{100},
            Player_Tag{}
        );
    }
    void Element::createGameState() const {
        Entity levelEntity = Entity::create();
        levelEntity.addAll(
            GameState_Tag{}, // marks this as the singleton level controller
            CurrentLevel{1}, // start at wave 0
            UIIntent{}    // starts at None
        );
    }
    void Element::createSpawnManager() const {
        Entity spawnEntity = Entity::create();
        spawnEntity.addAll(
            SpawnManager_Tag{}, // marks the singleton spawn controller
            SpawnState{
                -1, // waveIndex = 0
                0, // remaining = number of creeps to spawn
                0.f // timeLeft = spawn immediately
            }
        );
    }
    void Element::createCreep(float speed, int hp, int goldBounty, SDL_FRect spriteRect) const {
        Entity creepEntity = Entity::create();
        creepEntity.addAll(
            Transform{{TURNS[0].x, TURNS[0].y}, 0.f,},
            Drawable{spriteRect, {spriteRect.w * TEX_SCALE, spriteRect.h * TEX_SCALE}},
            WaypointIndex{1}, // head to waypoint #1
            Velocity{{0.f, 0.f}},
            Speed{speed},
            HP{hp, hp},
            Gold_Bounty{goldBounty},
            Creep_Tag{}
        );
    }
    void Element::createTower(float x, float y, float range, int healthDamage,
                             float fire_rate, SDL_FRect spriteRect) const {
        Entity creepEntity = Entity::create();
        creepEntity.addAll(
            Transform{{x, y}, 0.f},
            Drawable{spriteRect, {spriteRect.w * TEX_SCALE, spriteRect.h * TEX_SCALE}},
            Range {range},
            Damage {healthDamage},
            FireRate {fire_rate, 0.0f},
            Target {-1}
        );
    }
    void Element::createBullet(const SDL_FPoint &src, const SDL_FPoint &dst,
                               int damage, int targetId) const {
        // compute direction & travel time
        float dx = dst.x - src.x;
        float dy = dst.y - src.y;
        float dist = SDL_sqrtf(dx * dx + dy * dy);
        if (dist < 1e-4f) dist = 1.f;

        SDL_FPoint vel{
            dx / dist * BULLET_SPEED,
            dy / dist * BULLET_SPEED
        };

        float angDeg = SDL_atan2f(dy, dx) * RAD_TO_DEG;
        float travelTime = dist / BULLET_SPEED;

        // now spawn the bullet entity
        Entity b = Entity::create();
        b.addAll(
            Transform{src, angDeg},
            Drawable{BULLET_TEX, {BULLET_TEX.w * TEX_SCALE, BULLET_TEX.h * TEX_SCALE}},
            Velocity{vel},
            TravelTime{travelTime},
            Damage{damage},
            Target{targetId},
            Bullet_Tag{}
        );
    }
    // @formatter:on


    /// systems
    void Element::input_system() const {
        static const Mask mouseMask = MaskBuilder()
                .set<Mouse_Tag>()
                .set<MouseInput>()
                .set<Transform>()
                .build();

        auto mouseEnt = findEntity(mouseMask);
        if (mouseEnt.id == -1) return;

        auto &mi = World::getComponent<MouseInput>(mouseEnt);
        auto &t = World::getComponent<Transform>(mouseEnt);

        // 2) Clear last frame’s click
        mi.clicked = false;

        // 3) Poll SDL events and feed into that same mouseEnt
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) exit(0);

            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                mi.x = static_cast<int>(e.motion.x);
                mi.y = static_cast<int>(e.motion.y);
            } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                mi.x = static_cast<int>(e.button.x);
                mi.y = static_cast<int>(e.button.y);
                mi.clicked = true;
            }

            // always update the visual cursor position too
            t.p.x = static_cast<float>(mi.x);
            t.p.y = static_cast<float>(mi.y);
        }
    }

    void Element::ui_system() const {
        // 1) Read the one & only MouseInput entity
        static const Mask mouseMask = MaskBuilder()
                .set<Mouse_Tag>()
                .set<MouseInput>()
                .set<Transform>()
                .build();

        ent_type mouseEnt = findEntity(mouseMask);
        if (mouseEnt.id == -1) return;
        const auto &mi = World::getComponent<MouseInput>(mouseEnt);
        if (!mi.clicked) return; // only respond to an actual click

        // 2) Grab the single GameState (where UIIntent lives)
        static const Mask intentMask = MaskBuilder()
                .set<GameState_Tag>() // I assume this is your GameState tag
                .set<UIIntent>()
                .build();

        ent_type gs = findEntity(intentMask);
        if (gs.id == -1) return;

        auto &intent = World::getComponent<UIIntent>(gs);
        if (intent.action == UIAction::NextLevel)
            intent.action = UIAction::None; // reset before setting a new one

        // 3) Hit‐test every UIButton_Tag
        static const Mask btnMask = MaskBuilder()
                .set<UIButton_Tag>()
                .set<Transform>()
                .set<Drawable>()
                .build();

        for (ent_type b{0}; b.id <= World::maxId().id; ++b.id) {
            if (!World::mask(b).test(btnMask))
                continue;

            const auto &t = World::getComponent<Transform>(b);
            const auto &d = World::getComponent<Drawable>(b);
            // @formatter:off
            float left   = t.p.x - d.size.x/2;
            float right  = t.p.x + d.size.x/2;
            float top    = t.p.y - d.size.y/2;
            float bottom = t.p.y + d.size.y/2;

            if ( static_cast<float>(mi.x) < left
              || static_cast<float>(mi.x) > right
              || static_cast<float>(mi.y) < top
              || static_cast<float>(mi.y) > bottom )
                continue;

            const Mask &m = World::mask(b);
            if      (m.test(Component<Arrow_Tag>::Bit))       intent.action = UIAction::BuyArrow;
            else if (m.test(Component<Cannon_Tag>::Bit))      intent.action = UIAction::BuyCannon;
            else if (m.test(Component<Air_Tag>::Bit))         intent.action = UIAction::BuyAir;
            else if (m.test(Component<NextLevel_Tag>::Bit))   intent.action = UIAction::NextLevel;
            break;
            // @formatter:on
        }
    }

    void Element::path_navigation_system() const {
        static const Mask mask = MaskBuilder()
                .set<Transform>()
                .set<Speed>()
                .set<WaypointIndex>()
                .set<Velocity>()
                .set<Creep_Tag>()
                .build();

        constexpr float SNAP = 1.0f; // px distance considered “arrived”

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(mask))
                continue;

            auto &t = World::getComponent<Transform>(e);
            auto &wi = World::getComponent<WaypointIndex>(e);
            const auto &sp = World::getComponent<Speed>(e);
            auto &vel = World::getComponent<Velocity>(e);

            if (wi.idx >= TURN_COUNT)
                continue; // creep already at the end

            float wx = TURNS[wi.idx].x;
            float wy = TURNS[wi.idx].y;

            float dx = wx - t.p.x;
            float dy = wy - t.p.y;
            float distSq = dx * dx + dy * dy;

            if (distSq < SNAP * SNAP) {
                ++wi.idx; // snap & advance
                if (wi.idx >= TURN_COUNT) // reached base – handled elsewhere
                    continue;

                dx = TURNS[wi.idx].x - t.p.x;
                dy = TURNS[wi.idx].y - t.p.y;
            }

            float len = SDL_sqrtf(dx * dx + dy * dy);
            if (len < 1e-4f) len = 1.0f;

            vel.v = {
                dx / len * sp.value, // pixels per second
                dy / len * sp.value
            };

            t.a = RAD_TO_DEG * SDL_atan2f(vel.v.y, vel.v.x); // facing angle
        }
    }

    void Element::movement_system() const {
        // Only entities with a Transform and a Velocity move
        static const Mask mask = MaskBuilder()
                .set<Transform>()
                .set<Velocity>()
                .build();

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(mask))
                continue;

            auto &t = World::getComponent<Transform>(e);
            const auto &vel = World::getComponent<Velocity>(e);

            // DT is seconds per frame (1 / FPS)
            t.p.x += vel.v.x * DT;
            t.p.y += vel.v.y * DT;
        }
    }

    void Element::placing_tower_system() const {
        static const Mask mouseMask = MaskBuilder()
                .set<Mouse_Tag>()
                .set<MouseInput>()
                .set<Transform>()
                .set<Drawable>()
                .build();
        static const Mask intentMask = MaskBuilder()
                .set<GameState_Tag>()
                .set<UIIntent>()
                .build();

        // 1) Find mouse entity and UIIntent
        ent_type mouseEnt = findEntity(mouseMask);
        if (mouseEnt.id == -1) return;
        auto &mi = World::getComponent<MouseInput>(mouseEnt);
        auto &mouseD = World::getComponent<Drawable>(mouseEnt);

        ent_type gs = findEntity(intentMask);
        if (gs.id == -1) return;
        auto &intent = World::getComponent<UIIntent>(gs);

        // 2) If no buy-intent, clear any ghost sprite and bail
        if (intent.action == UIAction::None) {
            mouseD.part = SDL_FRect{}; // empty
            return;
        }

        // 3) Pick the correct tower sprite for the ghost
        SDL_FRect spriteRect;
        if (intent.action == UIAction::BuyArrow) spriteRect = TOWER_TEX_ARROW;
        else if (intent.action == UIAction::BuyCannon) spriteRect = TOWER_TEX_CANNON;
        else if (intent.action == UIAction::BuyAir) spriteRect = TOWER_TEX_AIR;
        else return;

        // 4) Attach ghost to mouse
        mouseD.part = spriteRect;
        mouseD.size = {spriteRect.w * TEX_SCALE, spriteRect.h * TEX_SCALE};

        // 5) On click, place real tower if inside map bounds
        if (mi.clicked) {
            float mx = mi.x, my = mi.y;
            // map rectangle in screen coords:
            float mapLeft = MAP_TEX_PAD_X;
            float mapRight = MAP_TEX_PAD_X + MAP_TEX.w * TEX_SCALE;
            float mapTop = MAP_TEX_PAD_Y;
            float mapBottom = MAP_TEX_PAD_Y + MAP_TEX.h * TEX_SCALE;


            if (mx >= mapLeft && mx <= mapRight && my >= mapTop && my <= mapBottom) {
                if (intent.action == UIAction::BuyArrow) {
                    createTower(mx, my, 200, 6, 0.5, spriteRect);
                } else if (intent.action == UIAction::BuyCannon) {
                    createTower(mx, my, 100, 10, 2, spriteRect);
                } else if (intent.action == UIAction::BuyAir) {
                    createTower(mx, my, 400, 3, 0.1, spriteRect);
                }

                intent.action = UIAction::None;
                // clear ghost
                mouseD.part = SDL_FRect{};
            }
        }
    }

    void Element::endpoint_system() const {
        // 1. Build masks
        static const Mask creepMask = MaskBuilder()
                .set<Creep_Tag>()
                .set<WaypointIndex>()
                .set<Transform>()
                .set<HP>()
                .set<Gold_Bounty>()
                .build();

        static const Mask playerMask = MaskBuilder()
                .set<Player_Tag>()
                .set<HP>()
                .set<Gold>()
                .build();

        // 2. Find the player entity (cache once)
        static ent_type player = findEntity(playerMask);
        if (player.id == -1) return; // no player found? bail

        auto &playerHP = World::getComponent<HP>(player);
        auto &playerGold = World::getComponent<Gold>(player);

        // 3. Process each creep that’s reached the end
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(creepMask))
                continue;

            auto &wi = World::getComponent<WaypointIndex>(e);
            auto &t = World::getComponent<Transform>(e);
            const auto &bounty = World::getComponent<Gold_Bounty>(e);

            if (wi.idx >= TURN_COUNT) {
                // a) Penalize the player
                playerHP.current = std::max(0, playerHP.current - 1);
                playerGold.current = std::max(0, playerGold.current - bounty.value);

                // b) Respawn the creep at the start
                t.p.x = TURNS[0].x;
                t.p.y = TURNS[0].y;
                wi.idx = 1; // head toward waypoint #1 next frame
            }
        }
    }

void Element::wave_system() const {
    // 1) Find SpawnManager singleton
    static const Mask mgrMask = MaskBuilder()
            .set<SpawnManager_Tag>()
            .set<SpawnState>()
            .build();
    ent_type mgr{-1};
    for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
        if (World::mask(e).test(mgrMask)) {
            mgr = e;
            break;
        }
    }
    if (mgr.id == -1) return;
    auto &st = World::getComponent<SpawnState>(mgr);

    // 2) If we're mid-spawning this wave, continue countdown + spawn
    if (st.remaining > 0) {
        const Wave &w = WAVES[st.waveIndex];
        st.timeLeft -= DT;
        if (st.timeLeft <= 0.f) {
            createCreep(w.speed, w.hp, w.gold, w.sprite);
            st.remaining -= 1;
            st.timeLeft = w.delay;
        }
        return;
    }

    // no new wave until current one’s creeps are all gone and player clicked
    // 3) Ensure no creeps remain alive before allowing next-wave click
    static const Mask creepMask = MaskBuilder().set<Creep_Tag>().build();
    for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
        if (World::mask(e).test(creepMask))
            return;
    }

    // 4) Handle NextLevel click to start the next wave
    static const Mask intentMask = MaskBuilder()
            .set<GameState_Tag>()
            .set<UIIntent>()
            .build();
    ent_type gs{-1};
    for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
        if (World::mask(e).test(intentMask)) {
            gs = e;
            break;
        }
    }
    if (gs.id == -1) return;
    auto &intent = World::getComponent<UIIntent>(gs);
    if (intent.action != UIAction::NextLevel)
        return;

    // 5) Advance and initialize next wave
    st.waveIndex += 1;
    if (st.waveIndex < WAVE_COUNT) {
        const Wave &w = WAVES[st.waveIndex];
        st.remaining = w.count;
        st.timeLeft = 0.f;

        // ─────────── update the displayed level ───────────
        static const Mask lvlMask = MaskBuilder()
                .set<GameState_Tag>()
                .set<CurrentLevel>()
                .build();
        ent_type levelEntity = findEntity(lvlMask);
        if (levelEntity.id != -1) {
            World::getComponent<CurrentLevel>(levelEntity).level = st.waveIndex + 1;
        }
    }

    // consume the UI intent
    intent.action = UIAction::None;
}


    void Element::draw_system() const {
        static const Mask mask = MaskBuilder()
                .set<Transform>() // where to draw
                .set<Drawable>() // what to draw
                .build();

        SDL_RenderClear(ren);

        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (!World::mask(e).test(mask))
                continue;

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

        print_status_bar();
        SDL_RenderPresent(ren);
    }

    void Element::drawScore(int score, float x, float y, float scale /*=1.0f*/) const {
        std::string s = std::to_string(score);
        float cx = x;
        for (char c: s) {
            // running on the string to create the num
            int d = c - '0';
            const SpriteFrame &frame = [&]() {
                switch (d) {
                    case 0: return sprite_digit_0;
                    case 1: return sprite_digit_1;
                    case 2: return sprite_digit_2;
                    case 3: return sprite_digit_3;
                    case 4: return sprite_digit_4;
                    case 5: return sprite_digit_5;
                    case 6: return sprite_digit_6;
                    case 7: return sprite_digit_7;
                    case 8: return sprite_digit_8;
                    case 9: return sprite_digit_9;
                    default: return sprite_digit_0;
                }
            }();


            SDL_FRect src;
            src.x = float(frame.x);
            src.y = float(frame.y);
            src.w = float(frame.w);
            src.h = float(frame.h);


            SDL_FRect dst;
            dst.x = cx;
            dst.y = y;
            dst.w = frame.w * scale;
            dst.h = frame.h * scale;

            SDL_RenderTexture(ren, digits, &src, &dst);
            cx += frame.w * scale; // spacing for next digit
        }
    }

    void Element::print_status_bar() const {
        // HP and Gold from player
        static const Mask uiMask = MaskBuilder()
                .set<Player_Tag>()
                .set<HP>()
                .set<Gold>()
                .build();
        ent_type player{-1};
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(uiMask)) {
                player = e;
                break;
            }
        }
        if (player.id == -1) return;
        int hp = World::getComponent<HP>(player).current;
        int gold = World::getComponent<Gold>(player).current;

        // Current level from GameState
        static const Mask lvlMask = MaskBuilder()
                .set<GameState_Tag>()
                .set<CurrentLevel>()
                .build();
        ent_type gs{-1};
        for (ent_type e{0}; e.id <= World::maxId().id; ++e.id) {
            if (World::mask(e).test(lvlMask)) {
                gs = e;
                break;
            }
        }
        int level = (gs.id != -1)
                        ? World::getComponent<CurrentLevel>(gs).level
                        : 0;

        // scale for displaying numbers
        const float scale = 0.4f;
        // placements of each info
        drawScore(hp, 800.f, 200.f, scale);
        drawScore(gold, 950.f, 200.f, scale);
        drawScore(level, 1200.f, 200.f, scale);
    }

    void Element::targeting_system() const {
        // Mask for towers that can target
        static const Mask towerMask = MaskBuilder()
                .set<Transform>()
                .set<Range>()
                .set<Target>()
                .build();

        // Mask for alive creeps with a known waypoint
        static const Mask creepMask = MaskBuilder()
                .set<Creep_Tag>()
                .set<Transform>()
                .set<WaypointIndex>()
                .build();

        // For each tower…
        for (ent_type t{0}; t.id <= World::maxId().id; ++t.id) {
            if (!World::mask(t).test(towerMask))
                continue;

            auto &tgt = World::getComponent<Target>(t);
            const auto &tp = World::getComponent<Transform>(t).p;
            float range = World::getComponent<Range>(t).value;
            float rangeSq = range * range;

            // 1) If we already have a target, check validity
            if (tgt.id != -1) {
                ent_type old{tgt.id};
                if (!World::mask(old).test(creepMask)) {
                    tgt.id = -1;
                } else {
                    const auto &cp = World::getComponent<Transform>(old).p;
                    float dx = cp.x - tp.x, dy = cp.y - tp.y;
                    if (dx * dx + dy * dy > rangeSq)
                        tgt.id = -1;
                }
            }

            // 2) If no valid target, search for the furthest‐along creep in range
            if (tgt.id == -1) {
                float bestScore = -1.0f;
                float bestSubDist = std::numeric_limits<float>::infinity();
                ent_type bestCreep = ent_type{-1};

                for (ent_type c{0}; c.id <= World::maxId().id; ++c.id) {
                    if (!World::mask(c).test(creepMask))
                        continue;

                    const auto &cp = World::getComponent<Transform>(c).p;
                    float dx = cp.x - tp.x, dy = cp.y - tp.y;
                    if (dx * dx + dy * dy > rangeSq)
                        continue;

                    int idx = World::getComponent<WaypointIndex>(c).idx;
                    float score = static_cast<float>(idx);

                    if (score > bestScore) {
                        // strictly better index
                        bestScore = score;
                        bestCreep = c;
                        // compute distance squared to the *next* waypoint
                        const auto &nx = TURNS[idx].x;
                        const auto &ny = TURNS[idx].y;
                        float sx = nx - cp.x, sy = ny - cp.y;
                        bestSubDist = sx * sx + sy * sy;
                    } else if (score == bestScore) {
                        // tie: pick the one closer to next waypoint
                        const auto &nx = TURNS[idx].x;
                        const auto &ny = TURNS[idx].y;
                        float sx = nx - cp.x, sy = ny - cp.y;
                        float subDist = sx * sx + sy * sy;
                        if (subDist < bestSubDist) {
                            bestSubDist = subDist;
                            bestCreep = c;
                        }
                    }
                }

                tgt.id = bestCreep.id;
            }
        }
    }

    void Element::shooting_system() const {
        static const Mask towerMask = MaskBuilder()
                .set<Transform>()
                .set<Damage>()
                .set<FireRate>()
                .set<Target>()
                .build();
        static const Mask creepMask = MaskBuilder()
                .set<Transform>()
                .set<Creep_Tag>()
                .build();

        for (ent_type t{0}; t.id <= World::maxId().id; ++t.id) {
            if (!World::mask(t).test(towerMask)) continue;

            auto &fr = World::getComponent<FireRate>(t);
            auto &tgt = World::getComponent<Target>(t);

            // 1) Cooldown tick
            if (fr.timeLeft > 0.f) {
                fr.timeLeft -= DT;
                continue;
            }
            // 2) No valid target?
            if (tgt.id == -1) continue;

            ent_type creep{tgt.id};
            if (!World::mask(creep).test(creepMask)) {
                tgt.id = -1;
                continue;
            }

            // 3) Gather parameters
            const auto &srcPt = World::getComponent<Transform>(t).p;
            const auto &dstPt = World::getComponent<Transform>(creep).p;
            int dmg = World::getComponent<Damage>(t).value;
            int tid = tgt.id;

            // 4) Spawn the bullet
            createBullet(srcPt, dstPt, dmg, tid);

            // 5) Reset the fire‐rate timer
            fr.timeLeft = fr.interval;
        }
    }

    void Element::homing_system() const {
        // Only bullets that still have a Target and a Velocity get updated
        static const Mask mask = MaskBuilder()
                .set<Bullet_Tag>()
                .set<Velocity>()
                .set<Target>()
                .build();
        static const Mask creepMask = MaskBuilder()
                .set<Creep_Tag>()
                .set<Transform>()
                .build();

        // Match the speed you used in createBullet()
        for (ent_type b{0}; b.id <= World::maxId().id; ++b.id) {
            if (!World::mask(b).test(mask)) continue;

            auto &tgt = World::getComponent<Target>(b);
            auto &vel = World::getComponent<Velocity>(b);
            const auto &src = World::getComponent<Transform>(b).p;

            // If the target’s gone or died, just destroy the bullet
            ent_type creep{tgt.id};
            if (!World::mask(creep).test(creepMask)) {
                World::destroyEntity(b);
                continue;
            }

            // Re-aim toward the creep’s current position
            const auto &dst = World::getComponent<Transform>(creep).p;
            float dx = dst.x - src.x;
            float dy = dst.y - src.y;
            float dist = SDL_sqrtf(dx * dx + dy * dy);
            if (dist < 1e-4f) dist = 1.f;

            vel.v = {
                dx / dist * BULLET_SPEED,
                dy / dist * BULLET_SPEED
            };
        }
    }

    void Element::bullet_hit_system() const {
        static const Mask mask = MaskBuilder()
                .set<Bullet_Tag>()
                .set<TravelTime>()
                .set<Target>()
                .set<Damage>()
                .build();

        static const Mask creepMask = MaskBuilder()
                .set<Creep_Tag>()
                .set<Transform>()
                .set<HP>()
                .build();

        for (ent_type b{0}; b.id <= World::maxId().id; ++b.id) {
            if (!World::mask(b).test(mask))
                continue;

            auto &tt = World::getComponent<TravelTime>(b);
            tt.travelTime -= DT;
            if (tt.travelTime > 0.f)
                continue; // not yet at center

            // now we’ve reached the creep’s center
            auto &tgt = World::getComponent<Target>(b);
            ent_type creep{tgt.id};
            if (World::mask(creep).test(creepMask)) {
                auto &hp = World::getComponent<HP>(creep);
                hp.current -= World::getComponent<Damage>(b).value;
                if (hp.current <= 0)
                    World::destroyEntity(creep);
            }
            World::destroyEntity(b);
        }
    }


    /// game
    Element::Element() {
        if (!prepareWindowAndTexture()) return;
        createUI();
        createPlayer();
        createMouse();
        createGameState();
        createSpawnManager();
    }

    Element::~Element() {
        if (tex != nullptr)
            SDL_DestroyTexture(tex);
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    [[noreturn]] void Element::run() {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        auto start = SDL_GetTicks();
        while (true) {
            input_system();
            ui_system();
            placing_tower_system();

            wave_system();
            path_navigation_system();
            endpoint_system();

            targeting_system();
            shooting_system();
            homing_system();
            // damage_system();
            bullet_hit_system();

            movement_system();
            draw_system();

            const auto end = SDL_GetTicks();
            if (const auto elapsed = end - start;
                static_cast<float>(elapsed) < GAME_FRAME) {
                SDL_Delay(static_cast<Uint32>(GAME_FRAME - static_cast<float>(elapsed)));
            }
            start += static_cast<Uint64>(GAME_FRAME);
        }
    }
}
