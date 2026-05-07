#pragma once
// Host-side stub for <Arduino.h>. Only the surface actually referenced by
// lib/ code that we want to build for the simulator is provided here.

#include <cassert>
#include <chrono>
#include <cmath>
#include <thread>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define IRAM_ATTR
#define DRAM_ATTR
#define RTC_NOINIT_ATTR
#define PROGMEM
#define F(x) (x)

inline unsigned long millis() {
  using namespace std::chrono;
  static const auto t0 = steady_clock::now();
  return static_cast<unsigned long>(
      duration_cast<milliseconds>(steady_clock::now() - t0).count());
}

inline unsigned long micros() {
  using namespace std::chrono;
  static const auto t0 = steady_clock::now();
  return static_cast<unsigned long>(
      duration_cast<microseconds>(steady_clock::now() - t0).count());
}

inline void delay(unsigned long ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Arduino String is used widely; alias to std::string for now. Most call
// sites use only ctor / c_str() / += / operator+, all available on std::string.
using String = std::string;

// Arduino exposes min/max as macros, but we don't want that on host.
