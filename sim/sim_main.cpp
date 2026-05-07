// Spike: drive GfxRenderer on host, blit framebuffer to an SDL window.
// Validates that the rendering pipeline (GfxRenderer + EpdFont) compiles and
// produces correct output without ESP-IDF / Arduino.

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "EpdFont.h"
#include "EpdFontFamily.h"
#include "FontDecompressor.h"
#include "FontCacheManager.h"
#include "GfxRenderer.h"
#include "HalDisplay.h"
#include "builtinFonts/notoserif_18_regular.h"
#include "builtinFonts/notoserif_18_bold.h"
#include "builtinFonts/notoserif_18_italic.h"
#include "builtinFonts/notoserif_18_bolditalic.h"

#include "HalGPIO.h"

HalDisplay display;
HalGPIO gpio;

namespace {

constexpr int FONT_BODY = 1;

// Convert the 1bpp framebuffer (MSB-first per byte) to a 32-bit ARGB texture.
void blitTo(SDL_Texture* tex, const uint8_t* fb, int w, int h) {
  uint32_t* pixels = nullptr;
  int pitch = 0;
  if (SDL_LockTexture(tex, nullptr, reinterpret_cast<void**>(&pixels), &pitch) != 0 ||
      !pixels) {
    return;  // Dummy driver / no surface; skip the blit.
  }
  const int wb = w / 8;
  for (int y = 0; y < h; ++y) {
    uint32_t* row = pixels + y * (pitch / 4);
    for (int x = 0; x < w; ++x) {
      const uint8_t byte = fb[y * wb + (x >> 3)];
      const bool on = byte & (0x80 >> (x & 7));
      row[x] = on ? 0xFFFFFFFFu : 0xFF202020u;  // white or near-black
    }
  }
  SDL_UnlockTexture(tex);
}

}  // namespace

int main(int argc, char** argv) {
  bool headless = false;
  for (int k = 1; k < argc; ++k) {
    if (std::strcmp(argv[k], "--headless") == 0) headless = true;
  }
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

  display.begin();

  GfxRenderer renderer(display);
  renderer.begin();
  // Native panel orientation = 800×480 logical; matches our spike layout.
  renderer.setOrientation(GfxRenderer::LandscapeCounterClockwise);

  // Wire up one font family so we can prove text rendering works.
  EpdFont r(&notoserif_18_regular);
  EpdFont b(&notoserif_18_bold);
  EpdFont i(&notoserif_18_italic);
  EpdFont bi(&notoserif_18_bolditalic);
  EpdFontFamily fam(&r, &b, &i, &bi);
  renderer.insertFont(FONT_BODY, fam);

  FontDecompressor fontDecompressor;
  FontCacheManager fontCache(renderer.getFontMap());
  renderer.setFontCacheManager(&fontCache);

  renderer.clearScreen();
  renderer.drawText(FONT_BODY, 40, 80,
                    "CrossPoint simulator spike — host SDL build", true);
  renderer.drawText(FONT_BODY, 40, 120,
                    "GfxRenderer + EpdFont link cleanly off-device.", true);
  renderer.drawRect(20, 20, 760, 440, 2, true);

  if (headless) {
    // Compute a quick checksum of the framebuffer so the smoke test asserts
    // that GfxRenderer actually wrote pixels (not just that it linked).
    const uint8_t* fb = display.getFrameBuffer();
    uint32_t hash = 0;
    for (uint32_t i = 0; i < HalDisplay::BUFFER_SIZE; ++i) {
      hash = hash * 31 + fb[i];
    }
    SDL_Quit();
    std::printf("spike: rendered %dx%d framebuffer; fb_hash=0x%08x\n",
                HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT, hash);
    return 0;
  }

  SDL_Window* win = SDL_CreateWindow(
      "CrossPoint Sim (spike)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_Renderer* sdl = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_Texture* tex = SDL_CreateTexture(
      sdl, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
      HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT);

  blitTo(tex, display.getFrameBuffer(), HalDisplay::DISPLAY_WIDTH,
         HalDisplay::DISPLAY_HEIGHT);

  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT ||
          (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
        running = false;
      }
    }
    SDL_RenderClear(sdl);
    SDL_RenderCopy(sdl, tex, nullptr, nullptr);
    SDL_RenderPresent(sdl);
    SDL_Delay(16);
  }

  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
