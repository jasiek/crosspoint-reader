#include "HalGPIO.h"

void HalGPIO::simSetButton(uint8_t buttonIndex, bool pressed) {
  if (buttonIndex >= BTN_COUNT) return;
  // Track when the *first* button transitioned to pressed so getHeldTime()
  // returns a sensible duration. Reset when everything is released.
  const bool wasAny = isPressed(BTN_BACK) || isPressed(BTN_CONFIRM) ||
                      isPressed(BTN_LEFT) || isPressed(BTN_RIGHT) ||
                      isPressed(BTN_UP) || isPressed(BTN_DOWN) ||
                      isPressed(BTN_POWER);
  current_[buttonIndex] = pressed;
  if (pressed && !wasAny) pressStartMs_ = millis();
  if (!pressed && wasAnyPressed() == false && wasAnyReleased() == false &&
      !current_[BTN_BACK] && !current_[BTN_CONFIRM] && !current_[BTN_LEFT] &&
      !current_[BTN_RIGHT] && !current_[BTN_UP] && !current_[BTN_DOWN] &&
      !current_[BTN_POWER]) {
    pressStartMs_ = 0;
  }
}

void HalGPIO::update() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) previous_[i] = current_[i];
}

bool HalGPIO::isPressed(uint8_t b) const {
  return b < BTN_COUNT && current_[b];
}

bool HalGPIO::wasPressed(uint8_t b) const {
  return b < BTN_COUNT && current_[b] && !previous_[b];
}

bool HalGPIO::wasReleased(uint8_t b) const {
  return b < BTN_COUNT && !current_[b] && previous_[b];
}

bool HalGPIO::wasAnyPressed() const {
  for (uint8_t i = 0; i < BTN_COUNT; ++i)
    if (current_[i] && !previous_[i]) return true;
  return false;
}

bool HalGPIO::wasAnyReleased() const {
  for (uint8_t i = 0; i < BTN_COUNT; ++i)
    if (!current_[i] && previous_[i]) return true;
  return false;
}

unsigned long HalGPIO::getHeldTime() const {
  if (pressStartMs_ == 0) return 0;
  return millis() - pressStartMs_;
}
