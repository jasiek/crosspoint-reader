#pragma once
// Minimal host stub for HalDisplay used by GfxRenderer in the simulator.
// Owns a single 1bpp framebuffer plus optional grayscale planes.
#include <Arduino.h>

#include <cstdint>
#include <cstring>

class HalDisplay {
 public:
  static constexpr uint16_t DISPLAY_WIDTH = 800;
  static constexpr uint16_t DISPLAY_HEIGHT = 480;
  static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
  static constexpr uint32_t BUFFER_SIZE = DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

  enum RefreshMode { FULL_REFRESH, HALF_REFRESH, FAST_REFRESH };

  HalDisplay() {
    std::memset(frameBuffer, 0xFF, BUFFER_SIZE);
  }

  void begin() {}

  uint8_t* getFrameBuffer() const { return const_cast<uint8_t*>(frameBuffer); }
  uint16_t getDisplayWidth() const { return DISPLAY_WIDTH; }
  uint16_t getDisplayHeight() const { return DISPLAY_HEIGHT; }
  uint16_t getDisplayWidthBytes() const { return DISPLAY_WIDTH_BYTES; }
  uint32_t getBufferSize() const { return BUFFER_SIZE; }

  void clearScreen(uint8_t color = 0xFF) const {
    std::memset(const_cast<uint8_t*>(frameBuffer), color, BUFFER_SIZE);
  }

  // Image draw: 1bpp bitmaps blitted at byte-aligned x. The simulator path is
  // best-effort — alignment edge cases match the device path closely enough
  // for visual validation.
  void drawImage(const uint8_t* imageData, uint16_t x, uint16_t y, uint16_t w,
                 uint16_t h, bool /*fromProgmem*/ = false) const {
    const uint16_t rowBytes = (w + 7) / 8;
    for (uint16_t row = 0; row < h; ++row) {
      const uint16_t dy = y + row;
      if (dy >= DISPLAY_HEIGHT) break;
      for (uint16_t col = 0; col < w; ++col) {
        const uint16_t dx = x + col;
        if (dx >= DISPLAY_WIDTH) break;
        const bool bit = imageData[row * rowBytes + (col >> 3)] &
                         (0x80 >> (col & 7));
        const uint32_t idx = dy * DISPLAY_WIDTH_BYTES + (dx >> 3);
        const uint8_t mask = 0x80 >> (dx & 7);
        if (bit)
          const_cast<uint8_t*>(frameBuffer)[idx] |= mask;
        else
          const_cast<uint8_t*>(frameBuffer)[idx] &= ~mask;
      }
    }
  }

  void drawImageTransparent(const uint8_t* imageData, uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            bool fromProgmem = false) const {
    drawImage(imageData, x, y, w, h, fromProgmem);
  }

  void displayBuffer(RefreshMode = FAST_REFRESH, bool = false) {}
  void refreshDisplay(RefreshMode = FAST_REFRESH, bool = false) {}
  void deepSleep() {}

  void copyGrayscaleBuffers(const uint8_t*, const uint8_t*) {}
  void copyGrayscaleLsbBuffers(const uint8_t*) {}
  void copyGrayscaleMsbBuffers(const uint8_t*) {}
  void cleanupGrayscaleBuffers(const uint8_t*) {}
  void displayGrayBuffer(bool = false) {}

 private:
  alignas(8) uint8_t frameBuffer[BUFFER_SIZE]{};
};

extern HalDisplay display;
