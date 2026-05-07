// Sim-side stub for RecentBooksStore. In-memory only.
#include "RecentBooksStore.h"

#include <algorithm>

RecentBooksStore RecentBooksStore::instance;

namespace JsonSettingsIO {
bool loadRecentBooks(RecentBooksStore& /*store*/, const char* /*json*/) {
  return false;
}
}  // namespace JsonSettingsIO

void RecentBooksStore::addBook(const std::string& path,
                               const std::string& title,
                               const std::string& author,
                               const std::string& coverBmpPath) {
  auto it = std::find_if(recentBooks.begin(), recentBooks.end(),
                         [&](const RecentBook& b) { return b.path == path; });
  if (it != recentBooks.end()) recentBooks.erase(it);
  recentBooks.insert(recentBooks.begin(),
                     RecentBook{path, title, author, coverBmpPath});
  if (recentBooks.size() > 16) recentBooks.resize(16);
}

void RecentBooksStore::updateBook(const std::string& path,
                                  const std::string& title,
                                  const std::string& author,
                                  const std::string& coverBmpPath) {
  for (auto& b : recentBooks) {
    if (b.path == path) {
      b.title = title;
      b.author = author;
      b.coverBmpPath = coverBmpPath;
      return;
    }
  }
}

bool RecentBooksStore::saveToFile() const { return true; }
bool RecentBooksStore::loadFromFile() { return false; }
bool RecentBooksStore::loadFromBinaryFile() { return false; }

RecentBook RecentBooksStore::getDataFromBook(std::string path) const {
  for (const auto& b : recentBooks)
    if (b.path == path) return b;
  return {};
}
