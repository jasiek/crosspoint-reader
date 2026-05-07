#pragma once
// Simulator-side HalPowerManager stub. Mirrors the public API of
// lib/hal/HalPowerManager.h enough for code that displays battery/sleep
// state. The real device backend speaks I2C to a fuel gauge or reads ADC;
// here we just return a fixed value.

#include <Arduino.h>
#include <HalGPIO.h>  // device header pulls this in transitively; mirror it

#include <cstdint>

class HalPowerManager {
 public:
  HalPowerManager() = default;

  void begin() {}
  void update() {}
  bool isCharging() const { return false; }
  bool isUsbPowered() const { return false; }
  bool isInLowPowerMode() const { return false; }
  void enterLowPowerMode() {}
  void exitLowPowerMode() {}
  void startDeepSleep(HalGPIO& /*gpio*/) const {}

  uint16_t getBatteryPercentage() const { return 80; }

  class Lock {
   public:
    Lock() = default;
    ~Lock() = default;
    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;
    Lock(Lock&&) = delete;
    Lock& operator=(Lock&&) = delete;
  };
};

extern HalPowerManager powerManager;
