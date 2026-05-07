// Sim-side stub for CrossPointSettings. Uses the real header so the public
// surface is identical, but skips JSON persistence — settings are pure
// in-memory defaults. When activities need to mutate settings the changes
// stick for the session and are discarded on exit.

#include "CrossPointSettings.h"

#include "fontIds.h"

CrossPointSettings CrossPointSettings::instance;

bool CrossPointSettings::saveToFile() const { return true; }
bool CrossPointSettings::loadFromFile() { return false; }
bool CrossPointSettings::loadFromBinaryFile() { return false; }
bool CrossPointSettings::migrateLanguageBinaryFile() { return false; }

uint8_t CrossPointSettings::writeSettings(FsFile& /*file*/,
                                          bool /*count_only*/) const {
  return 0;
}

void CrossPointSettings::validateFrontButtonMapping(
    CrossPointSettings& /*settings*/) {}

int CrossPointSettings::getReaderFontId() const {
  switch (fontFamily) {
    case NOTOSANS:
      switch (fontSize) {
        case SMALL: return NOTOSANS_12_FONT_ID;
        case MEDIUM: return NOTOSANS_14_FONT_ID;
        case LARGE: return NOTOSANS_16_FONT_ID;
        case EXTRA_LARGE: return NOTOSANS_18_FONT_ID;
      }
      break;
    case OPENDYSLEXIC:
      switch (fontSize) {
        case SMALL: return OPENDYSLEXIC_8_FONT_ID;
        case MEDIUM: return OPENDYSLEXIC_10_FONT_ID;
        case LARGE: return OPENDYSLEXIC_12_FONT_ID;
        case EXTRA_LARGE: return OPENDYSLEXIC_14_FONT_ID;
      }
      break;
    default:  // NOTOSERIF
      switch (fontSize) {
        case SMALL: return NOTOSERIF_12_FONT_ID;
        case MEDIUM: return NOTOSERIF_14_FONT_ID;
        case LARGE: return NOTOSERIF_16_FONT_ID;
        case EXTRA_LARGE: return NOTOSERIF_18_FONT_ID;
      }
      break;
  }
  return NOTOSERIF_14_FONT_ID;
}

float CrossPointSettings::getReaderLineCompression() const {
  switch (lineSpacing) {
    case TIGHT: return 0.85f;
    case WIDE: return 1.25f;
    default: return 1.0f;
  }
}

unsigned long CrossPointSettings::getSleepTimeoutMs() const {
  return 10UL * 60UL * 1000UL;  // 10 minutes default
}

int CrossPointSettings::getRefreshFrequency() const { return 15; }
