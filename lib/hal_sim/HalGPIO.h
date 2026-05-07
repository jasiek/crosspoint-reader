#pragma once
// Simulator-side HalGPIO. Mirrors the public API of lib/hal/HalGPIO.h so
// MappedInputManager and activity code work unmodified. Buttons are driven
// from the sim host loop via simSetButton(); update() snapshots the current
// state into "previous" so wasPressed/wasReleased fire on edges.

#include <Arduino.h>

#include <cstdint>

class HalGPIO {
 public:
  enum class DeviceType : uint8_t { X4, X3 };
  enum class WakeupReason { PowerButton, AfterFlash, AfterUSBPower, Other };

  static constexpr uint8_t BTN_BACK = 0;
  static constexpr uint8_t BTN_CONFIRM = 1;
  static constexpr uint8_t BTN_LEFT = 2;
  static constexpr uint8_t BTN_RIGHT = 3;
  static constexpr uint8_t BTN_UP = 4;
  static constexpr uint8_t BTN_DOWN = 5;
  static constexpr uint8_t BTN_POWER = 6;
  static constexpr uint8_t BTN_COUNT = 7;

  HalGPIO() = default;

  bool deviceIsX3() const { return false; }
  bool deviceIsX4() const { return true; }

  void begin() {}
  void update();

  bool isPressed(uint8_t buttonIndex) const;
  bool wasPressed(uint8_t buttonIndex) const;
  bool wasReleased(uint8_t buttonIndex) const;
  bool wasAnyPressed() const;
  bool wasAnyReleased() const;
  unsigned long getHeldTime() const;

  void startDeepSleep() {}
  void verifyPowerButtonWakeup(uint16_t /*requiredDurationMs*/,
                               bool /*shortPressAllowed*/) {}

  bool isUsbConnected() const { return false; }
  bool wasUsbStateChanged() const { return false; }

  WakeupReason getWakeupReason() const { return WakeupReason::Other; }

  // ---- Simulator-only -------------------------------------------------
  // Called by the sim host loop after translating an SDL key event into a
  // logical button index. Edge detection happens in update().
  void simSetButton(uint8_t buttonIndex, bool pressed);

 private:
  bool current_[BTN_COUNT] = {};
  bool previous_[BTN_COUNT] = {};
  unsigned long pressStartMs_ = 0;
};

extern HalGPIO gpio;
