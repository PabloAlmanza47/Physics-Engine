#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "physics.h"
#include <cstdlib>
#include <ctime>
#include <string>

// Draw a filled button with a centered text label
static void drawButton(SDL_Renderer* renderer, TTF_Font* font,
                       const SDL_Rect& rect, SDL_Color bg, const char* label) {
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &rect);

    // Subtle border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
    SDL_RenderDrawRect(renderer, &rect);

    // Centered label
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Blended(font, label, white);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst = { rect.x + (rect.w - surf->w) / 2, rect.y + (rect.h - surf->h) / 2, surf->w, surf->h };
        SDL_FreeSurface(surf);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

static void drawMenu(SDL_Renderer* renderer, TTF_Font* fontLarge, TTF_Font* fontSmall,
                     int winW, int winH,
                     const SDL_Rect& resumeBtn, const SDL_Rect& exitBtn) {
    // Dark translucent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, winW, winH};
    SDL_RenderFillRect(renderer, &full);

    // Panel
    int panelW = 300, panelH = 220;
    SDL_Rect panel = { (winW - panelW) / 2, (winH - panelH) / 2, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 20, 25, 40, 230);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 100, 180, 255, 80);
    SDL_RenderDrawRect(renderer, &panel);

    // "Paused" title
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Blended(fontLarge, "Paused", white);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst = { winW / 2 - surf->w / 2, panel.y + 20, surf->w, surf->h };
        SDL_FreeSurface(surf);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }

    SDL_Color resumeColor = { 60, 140, 220, 255 };
    SDL_Color exitColor   = { 180, 50,  50,  255 };
    drawButton(renderer, fontSmall, resumeBtn, resumeColor, "Resume");
    drawButton(renderer, fontSmall, exitBtn,   exitColor,   "Exit");
}

int main(int argc, char* argv[]) {
    srand((unsigned)time(nullptr));

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window*   window   = SDL_CreateWindow("Physics Sandbox",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* fontLarge = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    TTF_Font* fontSmall = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);

    PhysicsWorld world(800, 600);

    bool running  = true;
    bool menuOpen = false;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        // Button rects centered in window (recomputed each frame for resize support)
        SDL_Rect resumeBtn = { winW / 2 - 100, winH / 2 - 10, 200, 45 };
        SDL_Rect exitBtn   = { winW / 2 - 100, winH / 2 + 60, 200, 45 };

        Uint32 now = SDL_GetTicks();
        float dt   = std::min((now - lastTime) / 1000.0f, 0.033f);
        lastTime   = now;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED) {
                world.worldW = event.window.data1;
                world.worldH = event.window.data2;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    menuOpen = !menuOpen;  // ESC toggles the menu

                if (event.key.keysym.sym == SDLK_f && !menuOpen) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        SDL_SetWindowFullscreen(window, 0);
                    else
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;

                if (menuOpen) {
                    auto inRect = [](int x, int y, const SDL_Rect& r) {
                        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
                    };
                    if (inRect(mx, my, resumeBtn)) menuOpen = false;
                    if (inRect(mx, my, exitBtn))   running  = false;
                } else {
                    float size = 20.0f;
                    Body b(mx - size / 2.0f, my - size / 2.0f, size, size, 0.65f);
                    world.addBody(b);
                }
            }
        }

        // Pause simulation while menu is open
        if (!menuOpen)
            world.step(dt);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        for (const auto& b : world.bodies) {
            SDL_SetRenderDrawColor(renderer, 100, 180, 255, 255);
            SDL_Rect rect = { (int)b.pos.x, (int)b.pos.y, (int)b.w, (int)b.h };
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_RenderDrawRect(renderer, &rect);
        }

        if (menuOpen)
            drawMenu(renderer, fontLarge, fontSmall, winW, winH, resumeBtn, exitBtn);

        SDL_RenderPresent(renderer);
    }

    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontSmall) TTF_CloseFont(fontSmall);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}