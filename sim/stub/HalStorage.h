#pragma once
// Minimal HalStorage stub for the simulator spike. Mirrors only the surface
// transitively required by GfxRenderer/Bitmap/ZipFile headers. Real sim
// implementation will be std::filesystem-backed in lib/hal_sim/.
#include <Print.h>  // Device HalStorage.h pulls Print.h in transitively.
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
  size_t write(uint8_t b) {
    return fp_ && std::fputc(b, fp_) != EOF ? 1 : 0;
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

  // Resolve a logical path (like "/library/book.epub") against a host root
  // directory configured via SetSimRoot(). Defaults to "./sd-card".
  static void setSimRoot(const std::string& root) { simRoot() = root; }

  bool openFileForRead(const char* /*module*/, const char* path, FsFile& f) {
    return f.open(resolve(path).c_str(), 0);
  }
  bool openFileForRead(const char* m, const std::string& path, FsFile& f) {
    return openFileForRead(m, path.c_str(), f);
  }
  bool openFileForWrite(const char* m, const std::string& path, FsFile& f) {
    return openFileForWrite(m, path.c_str(), f);
  }
  bool openFileForWrite(const char* /*module*/, const char* path, FsFile& f) {
    f.close();
    auto resolved = resolve(path);
    auto* fp = std::fopen(resolved.c_str(), "wb");
    if (!fp) return false;
    f = FsFile{};
    // Tiny adopt: rather than expose internals, just close+reopen via API.
    std::fclose(fp);
    return f.open(resolved.c_str(), 0);  // FIXME: real impl needs r/w mode
  }
  bool exists(const char* path) {
    auto* fp = std::fopen(resolve(path).c_str(), "rb");
    if (fp) { std::fclose(fp); return true; }
    return false;
  }
  bool remove(const char* path) {
    return std::remove(resolve(path).c_str()) == 0;
  }
  bool mkdir(const char* /*path*/, bool /*pFlag*/ = true) { return true; }

 private:
  static std::string& simRoot() {
    static std::string r = "./sd-card";
    return r;
  }
  static std::string resolve(const char* path) {
    if (path && path[0] == '/') return simRoot() + path;
    return simRoot() + "/" + (path ? path : "");
  }
};

#define Storage HalStorage::getInstance()
