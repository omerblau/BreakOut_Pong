#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace game {
    using Transform = struct { SDL_FPoint p; float angle; };
    using Drawable  = struct { SDL_FRect part; SDL_FPoint size; SDL_Texture* tex; };
    using Intent    = struct { bool up, down, tilt_up, tilt_down; };
    using Collider  = struct { b2BodyId body;  };
    using Breakable = struct {};
    using Ball      = struct {};
}




