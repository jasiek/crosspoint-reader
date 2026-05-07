#pragma once
// Minimal HalGPIO stub for the simulator spike. The real lib/hal_sim/HalGPIO
// (Phase 2) maps SDL keyboard events to logical Button enums.
#include <Arduino.h>

#include <cstdint>

class HalGPIO {
 public:
  enum class DeviceType : uint8_t { X4, X3 };
  bool deviceIsX3() const { return false; }
  bool deviceIsX4() const { return true; }
};

extern HalGPIO gpio;
