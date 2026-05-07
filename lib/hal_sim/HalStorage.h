#pragma once
// Simulator-side HalStorage backed by std::filesystem. All paths are
// resolved against a sim "SD root" directory (default ./sd-card; override
// with HalStorage::setSimRoot() or env CROSSPOINT_SD_ROOT).
//
// Mirrors the device HalStorage public surface needed by current callers
// (Bitmap, ZipFile, CssParser, Epub, etc.). Methods that don't make sense on
// host (deepSleep, mutex begin) are no-ops.

#include <Print.h>  // matches device transitively-pulled header
#include <Arduino.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

class HalFile {
 public:
  HalFile() = default;
  ~HalFile() { close(); }
  HalFile(const HalFile&) = delete;
  HalFile& operator=(const HalFile&) = delete;
  HalFile(HalFile&& o) noexcept : fp_(o.fp_), writable_(o.writable_) {
    o.fp_ = nullptr;
  }
  HalFile& operator=(HalFile&& o) noexcept {
    if (&o != this) {
      close();
      fp_ = o.fp_;
      writable_ = o.writable_;
      o.fp_ = nullptr;
    }
    return *this;
  }

  bool open(const char* resolvedPath, int oflag = 0);
  void close();
  explicit operator bool() const { return fp_ != nullptr; }

  int read();
  size_t read(void* buf, size_t n);
  size_t write(uint8_t b);
  size_t write(const void* buf, size_t n);

  bool seek(uint32_t pos);
  bool seekCur(int32_t off);
  bool seekEnd(int32_t off = 0);
  uint32_t position();
  uint32_t size();
  bool available();

  // Adopt an already-opened FILE*; takes ownership.
  void adopt(std::FILE* fp, bool writable);

 private:
  std::FILE* fp_ = nullptr;
  bool writable_ = false;
};

// SdFat exposes its file type as `FsFile`; alias so existing call sites work.
using FsFile = HalFile;

class HalStorage {
 public:
  static HalStorage& getInstance() {
    static HalStorage inst;
    return inst;
  }

  static void setSimRoot(const std::string& root);
  static const std::string& simRoot();

  bool begin();
  bool ready() const { return true; }

  bool openFileForRead(const char* module, const char* path, FsFile& f);
  // On host, String is aliased to std::string (Arduino.h shim), so a single
  // std::string overload covers both API styles.
  bool openFileForRead(const char* module, const std::string& path,
                       FsFile& f) {
    return openFileForRead(module, path.c_str(), f);
  }

  bool openFileForWrite(const char* module, const char* path, FsFile& f);
  bool openFileForWrite(const char* module, const std::string& path,
                        FsFile& f) {
    return openFileForWrite(module, path.c_str(), f);
  }

  bool exists(const char* path);
  bool remove(const char* path);
  bool rename(const char* oldPath, const char* newPath);
  bool mkdir(const char* path, bool pFlag = true);
  bool rmdir(const char* path);
  bool removeDir(const char* path);
  bool ensureDirectoryExists(const char* path);

  std::vector<String> listFiles(const char* path = "/", int maxFiles = 200);

  std::string resolve(const char* path) const;
};

#define Storage HalStorage::getInstance()
