#pragma once
// Minimal HalStorage stub for the simulator spike. Mirrors only the surface
// transitively required by GfxRenderer/Bitmap headers. Real sim implementation
// will be std::filesystem-backed in lib/hal_sim/.
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

// SdFat's FsFile API surface used by Bitmap. Backed by FILE* on host.
class FsFile {
 public:
  FsFile() = default;
  ~FsFile() { close(); }
  bool open(const char* path, int /*oflag*/ = 0) {
    fp_ = std::fopen(path, "rb");
    return fp_ != nullptr;
  }
  void close() {
    if (fp_) { std::fclose(fp_); fp_ = nullptr; }
  }
  explicit operator bool() const { return fp_ != nullptr; }
  int read() { return fp_ ? std::fgetc(fp_) : -1; }
  size_t read(void* buf, size_t n) {
    return fp_ ? std::fread(buf, 1, n, fp_) : 0;
  }
  size_t write(const void* buf, size_t n) {
    return fp_ ? std::fwrite(buf, 1, n, fp_) : 0;
  }
  bool seek(uint32_t pos) { return fp_ && std::fseek(fp_, pos, SEEK_SET) == 0; }
  bool seekCur(int32_t off) { return fp_ && std::fseek(fp_, off, SEEK_CUR) == 0; }
  bool seekEnd(int32_t off = 0) { return fp_ && std::fseek(fp_, off, SEEK_END) == 0; }
  uint32_t position() { return fp_ ? std::ftell(fp_) : 0; }
  uint32_t size() {
    if (!fp_) return 0;
    long cur = std::ftell(fp_);
    std::fseek(fp_, 0, SEEK_END);
    long sz = std::ftell(fp_);
    std::fseek(fp_, cur, SEEK_SET);
    return static_cast<uint32_t>(sz);
  }
  bool available() {
    if (!fp_) return false;
    int c = std::fgetc(fp_);
    if (c == EOF) return false;
    std::ungetc(c, fp_);
    return true;
  }
 private:
  std::FILE* fp_ = nullptr;
};

using HalFile = FsFile;

class HalStorage {
 public:
  static HalStorage& getInstance() { static HalStorage inst; return inst; }
};

#define Storage HalStorage::getInstance()
