// CrossPoint simulator entry point.
//
// Drives the firmware's HAL (lib/hal_sim/) on the host with an SDL window
// and keyboard input. Demonstrates the canonical device flow:
//   - SDL key events feed HalGPIO::simSetButton(BTN_*) so wasPressed/
//     wasReleased fire on edges in HalGPIO::update().
//   - Application logic queries HalGPIO::wasPressed(BTN_*) like on device.
//   - Application calls display.displayBuffer() to publish a frame; the host
//     loop polls display.getRefreshTick() to decide when to re-blit.
//
// Sim-only host shortcuts (Esc / O / F12) bypass HalGPIO entirely.
//
// Keys: Up/Down move selection, X/Enter confirm, Z back, [/] page-side,
// P power button, O cycle orientation, F12 screenshot, Esc quit.

#include <SDL2/SDL.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "EpdFont.h"
#include "EpdFontFamily.h"
#include "FontDecompressor.h"
#include "FontCacheManager.h"
#include "GfxRenderer.h"
#include "HalDisplay.h"
#include "HalGPIO.h"
#include "HalStorage.h"
#include "diag/DiagFont.h"
#include "input/SimInput.h"
#include "builtinFonts/notoserif_18_regular.h"
#include "builtinFonts/notoserif_18_bold.h"
#include "builtinFonts/notoserif_18_italic.h"
#include "builtinFonts/notoserif_18_bolditalic.h"
#include "builtinFonts/notoserif_14_regular.h"
#include "builtinFonts/notoserif_14_bold.h"
#include "builtinFonts/notoserif_14_italic.h"
#include "builtinFonts/notoserif_14_bolditalic.h"

HalDisplay display;
HalGPIO gpio;

namespace {

constexpr int FONT_BODY = 1;
constexpr int FONT_SMALL = 2;

constexpr GfxRenderer::Orientation kOrientations[] = {
    GfxRenderer::LandscapeCounterClockwise,
    GfxRenderer::Portrait,
    GfxRenderer::LandscapeClockwise,
    GfxRenderer::PortraitInverted,
};

const char* orientationName(GfxRenderer::Orientation o) {
  switch (o) {
    case GfxRenderer::Portrait: return "PORTRAIT";
    case GfxRenderer::PortraitInverted: return "PORTRAIT INV";
    case GfxRenderer::LandscapeClockwise: return "LANDSCAPE CW";
    case GfxRenderer::LandscapeCounterClockwise: return "LANDSCAPE CCW";
  }
  return "?";
}

void blitFramebuffer(SDL_Texture* tex, const uint8_t* fb, int wPanel,
                     int hPanel) {
  uint32_t* pixels = nullptr;
  int pitch = 0;
  if (SDL_LockTexture(tex, nullptr, reinterpret_cast<void**>(&pixels),
                      &pitch) != 0 ||
      !pixels) {
    return;
  }
  const int wb = wPanel / 8;
  for (int y = 0; y < hPanel; ++y) {
    uint32_t* row = pixels + y * (pitch / 4);
    for (int x = 0; x < wPanel; ++x) {
      const uint8_t byte = fb[y * wb + (x >> 3)];
      const bool on = byte & (0x80 >> (x & 7));
      row[x] = on ? 0xFFEEEAE0u : 0xFF1A1A1Au;
    }
  }
  SDL_UnlockTexture(tex);
}

bool savePgmScreenshot(const uint8_t* fb, int wPanel, int hPanel,
                       const char* path) {
  std::FILE* f = std::fopen(path, "wb");
  if (!f) return false;
  std::fprintf(f, "P5\n%d %d\n255\n", wPanel, hPanel);
  const int wb = wPanel / 8;
  for (int y = 0; y < hPanel; ++y) {
    for (int x = 0; x < wPanel; ++x) {
      const uint8_t byte = fb[y * wb + (x >> 3)];
      const uint8_t v = (byte & (0x80 >> (x & 7))) ? 0xEE : 0x1A;
      std::fputc(v, f);
    }
  }
  std::fclose(f);
  return true;
}

struct Menu {
  std::array<const char*, 5> items{
      "Open library",
      "Recently read",
      "Settings",
      "About",
      "Power off (sim quit)",
  };
  int selected = 0;
  bool dirty = true;

  void up() { selected = (selected - 1 + items.size()) % items.size(); dirty = true; }
  void down() { selected = (selected + 1) % items.size(); dirty = true; }
  bool selectedIsQuit() const {
    return selected == static_cast<int>(items.size()) - 1;
  }
};

void renderMenu(GfxRenderer& renderer, const Menu& menu) {
  renderer.clearScreen();
  const int w = renderer.getScreenWidth();

  renderer.drawText(FONT_BODY, 30, 50, "CrossPoint Reader (sim)", true);
  renderer.drawLine(30, 78, w - 30, 78, true);

  int y = 120;
  for (size_t i = 0; i < menu.items.size(); ++i) {
    const bool selected = (static_cast<int>(i) == menu.selected);
    if (selected) {
      renderer.drawRect(30, y - 22, w - 60, 36, true);
      char buf[80];
      std::snprintf(buf, sizeof(buf), "> %s", menu.items[i]);
      renderer.drawText(FONT_BODY, 50, y, buf, false);
    } else {
      renderer.drawText(FONT_BODY, 50, y, menu.items[i], true);
    }
    y += 44;
  }
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
  gpio.begin();
  Storage.begin();

  GfxRenderer renderer(display);
  renderer.begin();
  size_t orientationIdx = 0;
  renderer.setOrientation(kOrientations[orientationIdx]);

  EpdFont r18(&notoserif_18_regular), b18(&notoserif_18_bold),
      i18(&notoserif_18_italic), bi18(&notoserif_18_bolditalic);
  EpdFontFamily fam18(&r18, &b18, &i18, &bi18);
  EpdFont r14(&notoserif_14_regular), b14(&notoserif_14_bold),
      i14(&notoserif_14_italic), bi14(&notoserif_14_bolditalic);
  EpdFontFamily fam14(&r14, &b14, &i14, &bi14);
  renderer.insertFont(FONT_BODY, fam18);
  renderer.insertFont(FONT_SMALL, fam14);

  static FontDecompressor fontDecompressor;
  static FontCacheManager fontCache(renderer.getFontMap());
  fontCache.setFontDecompressor(&fontDecompressor);
  renderer.setFontCacheManager(&fontCache);

  Menu menu;
  renderMenu(renderer, menu);
  display.displayBuffer();  // initial publish

  if (headless) {
    const uint8_t* fb = display.getFrameBuffer();
    uint32_t hash = 0;
    for (uint32_t i = 0; i < HalDisplay::BUFFER_SIZE; ++i) {
      hash = hash * 31 + fb[i];
    }
    const char* dumpPath = "sim_headless.pgm";
    savePgmScreenshot(fb, HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT,
                      dumpPath);
    SDL_Quit();
    std::printf(
        "sim: rendered %dx%d (orient=%s, refreshTick=%u); fb_hash=0x%08x; "
        "wrote %s\n",
        HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT,
        orientationName(kOrientations[orientationIdx]),
        display.getRefreshTick(), hash, dumpPath);
    return 0;
  }

  constexpr int kDiagH = 12;
  const int winW = HalDisplay::DISPLAY_WIDTH;
  const int winH = HalDisplay::DISPLAY_HEIGHT + kDiagH;

  SDL_Window* win = SDL_CreateWindow("CrossPoint Sim", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, winW, winH,
                                     SDL_WINDOW_SHOWN);
  SDL_Renderer* sdl = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_Texture* tex = SDL_CreateTexture(
      sdl, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
      HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT);

  uint32_t lastBlitTick = 0;
  bool running = true;
  int screenshotIdx = 0;

  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
        continue;
      }
      if (e.type != SDL_KEYDOWN && e.type != SDL_KEYUP) continue;
      const SDL_Keycode k = e.key.keysym.sym;

      // Host-only shortcuts fire on key-down only.
      if (e.type == SDL_KEYDOWN && !e.key.repeat) {
        switch (sim::hostActionForKey(k)) {
          case sim::HostAction::Quit:
            running = false;
            continue;
          case sim::HostAction::CycleOrientation:
            orientationIdx = (orientationIdx + 1) %
                             (sizeof(kOrientations) / sizeof(kOrientations[0]));
            renderer.setOrientation(kOrientations[orientationIdx]);
            menu.dirty = true;
            continue;
          case sim::HostAction::Screenshot: {
            char path[64];
            std::snprintf(path, sizeof(path), "screenshot_%04d.pgm",
                          screenshotIdx++);
            if (savePgmScreenshot(display.getFrameBuffer(),
                                  HalDisplay::DISPLAY_WIDTH,
                                  HalDisplay::DISPLAY_HEIGHT, path)) {
              std::printf("sim: wrote %s\n", path);
            }
            continue;
          }
          case sim::HostAction::None:
            break;
        }
      }

      // Map to a HalGPIO button, ignoring auto-repeat.
      const int8_t btn = sim::buttonForKey(k);
      if (btn < 0) continue;
      if (e.type == SDL_KEYDOWN && e.key.repeat) continue;
      gpio.simSetButton(static_cast<uint8_t>(btn), e.type == SDL_KEYDOWN);
    }

    gpio.update();

    // ---- Application "tick" — uses canonical HalGPIO API ----
    if (gpio.wasPressed(HalGPIO::BTN_UP)) menu.up();
    if (gpio.wasPressed(HalGPIO::BTN_DOWN)) menu.down();
    if (gpio.wasPressed(HalGPIO::BTN_CONFIRM) && menu.selectedIsQuit()) {
      running = false;
    }

    if (menu.dirty) {
      renderMenu(renderer, menu);
      display.displayBuffer();  // bumps refreshTick → host loop will blit
      menu.dirty = false;
    }

    if (display.getRefreshTick() != lastBlitTick) {
      blitFramebuffer(tex, display.getFrameBuffer(),
                      HalDisplay::DISPLAY_WIDTH, HalDisplay::DISPLAY_HEIGHT);
      lastBlitTick = display.getRefreshTick();
    }

    SDL_Rect panelDst{0, 0, HalDisplay::DISPLAY_WIDTH,
                      HalDisplay::DISPLAY_HEIGHT};
    SDL_RenderClear(sdl);
    SDL_RenderCopy(sdl, tex, nullptr, &panelDst);

    SDL_Rect diagBar{0, HalDisplay::DISPLAY_HEIGHT, winW, kDiagH};
    SDL_SetRenderDrawColor(sdl, 0, 0, 0, 255);
    SDL_RenderFillRect(sdl, &diagBar);
    char diag[96];
    std::snprintf(diag, sizeof(diag), "%s  REFRESH %u  HEAP %uK",
                  orientationName(kOrientations[orientationIdx]),
                  static_cast<unsigned>(display.getRefreshTick()),
                  static_cast<unsigned>(ESP.getFreeHeap() / 1024));
    sim::diag::drawText(sdl, 4, HalDisplay::DISPLAY_HEIGHT + 2, diag, 255, 255,
                        255);

    SDL_RenderPresent(sdl);
    SDL_Delay(16);
  }

  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
