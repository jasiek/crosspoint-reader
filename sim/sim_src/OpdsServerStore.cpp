// Sim-side stub for OpdsServerStore. In-memory, no persistence.
#include "OpdsServerStore.h"

OpdsServerStore OpdsServerStore::instance;

namespace JsonSettingsIO {
bool saveOpds(const OpdsServerStore&, const char*) { return true; }
bool loadOpds(OpdsServerStore&, const char*, bool*) { return false; }
}  // namespace JsonSettingsIO

bool OpdsServerStore::saveToFile() const { return true; }
bool OpdsServerStore::loadFromFile() { return false; }

bool OpdsServerStore::addServer(const OpdsServer& server) {
  if (servers.size() >= MAX_SERVERS) return false;
  servers.push_back(server);
  return true;
}

bool OpdsServerStore::updateServer(size_t index, const OpdsServer& server) {
  if (index >= servers.size()) return false;
  servers[index] = server;
  return true;
}

bool OpdsServerStore::removeServer(size_t index) {
  if (index >= servers.size()) return false;
  servers.erase(servers.begin() + index);
  return true;
}

const OpdsServer* OpdsServerStore::getServer(size_t index) const {
  if (index >= servers.size()) return nullptr;
  return &servers[index];
}

bool OpdsServerStore::migrateFromSettings() { return false; }
