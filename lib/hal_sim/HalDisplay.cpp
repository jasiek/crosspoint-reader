#include "HalDisplay.h"

#include <cstring>

HalDisplay::HalDisplay() { std::memset(frameBuffer_, 0xFF, BUFFER_SIZE); }

void HalDisplay::begin() {}

uint8_t* HalDisplay::getFrameBuffer() const {
  return const_cast<uint8_t*>(frameBuffer_);
}

void HalDisplay::clearScreen(uint8_t color) const {
  std::memset(const_cast<uint8_t*>(frameBuffer_), color, BUFFER_SIZE);
}

namespace {
inline void blitBit(uint8_t* fb, uint16_t dx, uint16_t dy, bool on) {
  if (dx >= HalDisplay::DISPLAY_WIDTH || dy >= HalDisplay::DISPLAY_HEIGHT)
    return;
  const uint32_t idx = dy * HalDisplay::DISPLAY_WIDTH_BYTES + (dx >> 3);
  const uint8_t mask = 0x80 >> (dx & 7);
  if (on)
    fb[idx] |= mask;
  else
    fb[idx] &= ~mask;
}
}  // namespace

void HalDisplay::drawImage(const uint8_t* imageData, uint16_t x, uint16_t y,
                           uint16_t w, uint16_t h, bool /*fromProgmem*/) const {
  uint8_t* fb = const_cast<uint8_t*>(frameBuffer_);
  const uint16_t rowBytes = (w + 7) / 8;
  for (uint16_t row = 0; row < h; ++row) {
    for (uint16_t col = 0; col < w; ++col) {
      const bool bit =
          imageData[row * rowBytes + (col >> 3)] & (0x80 >> (col & 7));
      blitBit(fb, x + col, y + row, bit);
    }
  }
}

void HalDisplay::drawImageTransparent(const uint8_t* imageData, uint16_t x,
                                      uint16_t y, uint16_t w, uint16_t h,
                                      bool fromProgmem) const {
  drawImage(imageData, x, y, w, h, fromProgmem);
}

void HalDisplay::displayBuffer(RefreshMode mode, bool /*turnOffScreen*/) {
  lastRefresh_ = mode;
  ++refreshTick_;
}

void HalDisplay::refreshDisplay(RefreshMode mode, bool turnOffScreen) {
  displayBuffer(mode, turnOffScreen);
}

void HalDisplay::deepSleep() {}

void HalDisplay::copyGrayscaleBuffers(const uint8_t*, const uint8_t*) {}
void HalDisplay::copyGrayscaleLsbBuffers(const uint8_t*) {}
void HalDisplay::copyGrayscaleMsbBuffers(const uint8_t*) {}
void HalDisplay::cleanupGrayscaleBuffers(const uint8_t*) {}
void HalDisplay::displayGrayBuffer(bool /*turnOffScreen*/) { ++refreshTick_; }
