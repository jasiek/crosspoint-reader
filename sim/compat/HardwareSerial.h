#pragma once
// Host stub for HardwareSerial / HWCDC, sufficient for Logging.h.
// On device, <Arduino.h> is transitively pulled in via this header and brings
// cstring/cstdarg/cstdio with it. Mirror that here.
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <Arduino.h>  // millis(), String, IRAM_ATTR, RTC_NOINIT_ATTR, ...
#include <Print.h>    // Logging.h's MySerialImpl extends Print.

class HWCDC {
 public:
  operator bool() const { return true; }  // implicit, matches HWCDC on device
  void begin(unsigned long /*baud*/ = 115200) {}
  void print(const char* s) { std::fputs(s, stdout); }
  void println(const char* s) { std::fputs(s, stdout); std::fputc('\n', stdout); }
  size_t write(uint8_t b) { std::fputc(b, stdout); return 1; }
  size_t write(const uint8_t* data, size_t len) {
    return std::fwrite(data, 1, len, stdout);
  }
  template <typename... Args>
  int printf(const char* fmt, Args&&... args) {
    return std::printf(fmt, args...);
  }
  void flush() { std::fflush(stdout); }
};

inline HWCDC Serial;
