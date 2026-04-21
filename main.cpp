#include <SDL2/SDL.h>
#include "physics.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// Color boxes by speed: blue (slow) → red (fast)
SDL_Color speedColor(const Body& b) {
    float speed = std::sqrt(b.vel.x * b.vel.x + b.vel.y * b.vel.y);
    float t = std::min(speed / 800.0f, 1.0f);
    return { (Uint8)(50 + t * 200), (Uint8)(120 - t * 100), (Uint8)(255 - t * 230), 255 };
}

int main(int argc, char* argv[]) {
    srand((unsigned)time(nullptr));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*   window   = SDL_CreateWindow("Physics Sandbox",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    PhysicsWorld world(800, 600);

    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt   = std::min((now - lastTime) / 1000.0f, 0.033f);  // cap at ~30fps min
        lastTime   = now;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            // Left click: spawn a new box at the cursor
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                float size = 20.0f;
                Body b(event.button.x - size / 2.0f, event.button.y - size / 2.0f, size, size, 0.65f);
                world.addBody(b);
            }
        }

        world.step(dt);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        for (const auto& b : world.bodies) {
            // Filled box colored by speed
            SDL_Color c = speedColor(b);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_Rect rect = { (int)b.pos.x, (int)b.pos.y, (int)b.w, (int)b.h };
            SDL_RenderFillRect(renderer, &rect);

            // Subtle white outline
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_RenderDrawRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}