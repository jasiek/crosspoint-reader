#include "HalStorage.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace {
std::string& root() {
  static std::string r = []() {
    if (const char* env = std::getenv("CROSSPOINT_SD_ROOT")) return std::string(env);
    return std::string("./sd-card");
  }();
  return r;
}
}  // namespace

// ---- HalFile ---------------------------------------------------------------

bool HalFile::open(const char* resolvedPath, int /*oflag*/) {
  close();
  fp_ = std::fopen(resolvedPath, "rb");
  writable_ = false;
  return fp_ != nullptr;
}

void HalFile::close() {
  if (fp_) {
    std::fclose(fp_);
    fp_ = nullptr;
  }
}

void HalFile::adopt(std::FILE* fp, bool writable) {
  close();
  fp_ = fp;
  writable_ = writable;
}

int HalFile::read() { return fp_ ? std::fgetc(fp_) : -1; }

size_t HalFile::read(void* buf, size_t n) {
  return fp_ ? std::fread(buf, 1, n, fp_) : 0;
}

size_t HalFile::write(uint8_t b) {
  return fp_ && std::fputc(b, fp_) != EOF ? 1 : 0;
}

size_t HalFile::write(const void* buf, size_t n) {
  return fp_ ? std::fwrite(buf, 1, n, fp_) : 0;
}

bool HalFile::seek(uint32_t pos) {
  return fp_ && std::fseek(fp_, static_cast<long>(pos), SEEK_SET) == 0;
}

bool HalFile::seekCur(int32_t off) {
  return fp_ && std::fseek(fp_, off, SEEK_CUR) == 0;
}

bool HalFile::seekEnd(int32_t off) {
  return fp_ && std::fseek(fp_, off, SEEK_END) == 0;
}

uint32_t HalFile::position() {
  return fp_ ? static_cast<uint32_t>(std::ftell(fp_)) : 0;
}

uint32_t HalFile::size() {
  if (!fp_) return 0;
  long cur = std::ftell(fp_);
  std::fseek(fp_, 0, SEEK_END);
  long sz = std::ftell(fp_);
  std::fseek(fp_, cur, SEEK_SET);
  return sz < 0 ? 0 : static_cast<uint32_t>(sz);
}

bool HalFile::available() {
  if (!fp_) return false;
  int c = std::fgetc(fp_);
  if (c == EOF) return false;
  std::ungetc(c, fp_);
  return true;
}

// ---- HalStorage ------------------------------------------------------------

void HalStorage::setSimRoot(const std::string& r) { root() = r; }
const std::string& HalStorage::simRoot() { return root(); }

bool HalStorage::begin() {
  std::error_code ec;
  fs::create_directories(root(), ec);
  return !ec;
}

std::string HalStorage::resolve(const char* path) const {
  if (!path || !*path) return root();
  if (path[0] == '/') return root() + path;
  return root() + "/" + path;
}

bool HalStorage::openFileForRead(const char* /*module*/, const char* path,
                                 FsFile& f) {
  return f.open(resolve(path).c_str(), 0);
}

bool HalStorage::openFileForWrite(const char* /*module*/, const char* path,
                                  FsFile& f) {
  auto resolved = resolve(path);
  std::error_code ec;
  fs::create_directories(fs::path(resolved).parent_path(), ec);
  std::FILE* fp = std::fopen(resolved.c_str(), "wb+");
  if (!fp) return false;
  f.adopt(fp, /*writable=*/true);
  return true;
}

bool HalStorage::exists(const char* path) {
  std::error_code ec;
  return fs::exists(resolve(path), ec);
}

bool HalStorage::remove(const char* path) {
  std::error_code ec;
  return fs::remove(resolve(path), ec);
}

bool HalStorage::rename(const char* oldPath, const char* newPath) {
  std::error_code ec;
  fs::rename(resolve(oldPath), resolve(newPath), ec);
  return !ec;
}

bool HalStorage::mkdir(const char* path, bool /*pFlag*/) {
  std::error_code ec;
  return fs::create_directories(resolve(path), ec);
}

bool HalStorage::rmdir(const char* path) {
  std::error_code ec;
  return fs::remove(resolve(path), ec);
}

bool HalStorage::removeDir(const char* path) {
  std::error_code ec;
  return fs::remove_all(resolve(path), ec) > 0;
}

bool HalStorage::ensureDirectoryExists(const char* path) {
  return mkdir(path);
}

std::vector<String> HalStorage::listFiles(const char* path, int maxFiles) {
  std::vector<String> out;
  std::error_code ec;
  fs::directory_iterator it(resolve(path), ec);
  if (ec) return out;
  for (const auto& e : it) {
    if (static_cast<int>(out.size()) >= maxFiles) break;
    out.emplace_back(e.path().filename().string());
  }
  return out;
}
