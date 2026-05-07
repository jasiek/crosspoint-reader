#pragma once
// Simulator-side HalDisplay. Mirrors the public API of lib/hal/HalDisplay.h
// so application code (GfxRenderer, activities) is binary-compatible across
// device and sim builds. Selected by include-path ordering when CROSSPOINT_EMULATED=1.

#include <Arduino.h>

#include <cstdint>

class HalDisplay {
 public:
  enum RefreshMode { FULL_REFRESH, HALF_REFRESH, FAST_REFRESH };

  static constexpr uint16_t DISPLAY_WIDTH = 800;
  static constexpr uint16_t DISPLAY_HEIGHT = 480;
  static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
  static constexpr uint32_t BUFFER_SIZE =
      DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

  HalDisplay();
  ~HalDisplay() = default;

  void begin();

  void clearScreen(uint8_t color = 0xFF) const;
  void drawImage(const uint8_t* imageData, uint16_t x, uint16_t y, uint16_t w,
                 uint16_t h, bool fromProgmem = false) const;
  void drawImageTransparent(const uint8_t* imageData, uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            bool fromProgmem = false) const;

  void displayBuffer(RefreshMode mode = FAST_REFRESH,
                     bool turnOffScreen = false);
  void refreshDisplay(RefreshMode mode = FAST_REFRESH,
                      bool turnOffScreen = false);

  void deepSleep();

  uint8_t* getFrameBuffer() const;

  void copyGrayscaleBuffers(const uint8_t* lsbBuffer,
                            const uint8_t* msbBuffer);
  void copyGrayscaleLsbBuffers(const uint8_t* lsbBuffer);
  void copyGrayscaleMsbBuffers(const uint8_t* msbBuffer);
  void cleanupGrayscaleBuffers(const uint8_t* bwBuffer);

  void displayGrayBuffer(bool turnOffScreen = false);

  uint16_t getDisplayWidth() const { return DISPLAY_WIDTH; }
  uint16_t getDisplayHeight() const { return DISPLAY_HEIGHT; }
  uint16_t getDisplayWidthBytes() const { return DISPLAY_WIDTH_BYTES; }
  uint32_t getBufferSize() const { return BUFFER_SIZE; }

  // ---- Simulator-only extensions ----------------------------------------
  // Monotonic counter; bumped every time displayBuffer/refreshDisplay/
  // displayGrayBuffer is called. The sim's host loop polls this to decide
  // when to re-blit the SDL texture (instead of every frame).
  uint32_t getRefreshTick() const { return refreshTick_; }

  // Last refresh mode requested. Lets the host visualise the difference
  // between full and partial refreshes (useful when emulating ghosting).
  RefreshMode getLastRefreshMode() const { return lastRefresh_; }

 private:
  alignas(8) uint8_t frameBuffer_[BUFFER_SIZE]{};
  uint32_t refreshTick_ = 0;
  RefreshMode lastRefresh_ = FAST_REFRESH;
};

extern HalDisplay display;
