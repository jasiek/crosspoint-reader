// Sim-side UITheme stub. The real device implementation manages a registry
// of three themes (Lyra, Lyra3Covers, RoundedRaff) and pulls in heavier
// dependencies. Sim hard-codes Lyra metrics and provides minimal answers
// for the static helpers BaseTheme/LyraTheme call back into.

#include "components/UITheme.h"

#include <string>

#include "components/themes/lyra/LyraTheme.h"

UITheme UITheme::instance;

UITheme::UITheme()
    : currentMetrics(&LyraMetrics::values),
      currentTheme(std::make_unique<LyraTheme>()) {}

void UITheme::reload() {}
void UITheme::setTheme(CrossPointSettings::UI_THEME /*type*/) {}

int UITheme::getNumberOfItemsPerPage(const GfxRenderer& /*renderer*/,
                                     bool /*hasHeader*/, bool /*hasTabBar*/,
                                     bool /*hasButtonHints*/,
                                     bool /*hasSubtitle*/,
                                     int /*extraReservedHeight*/) {
  return 8;
}

std::string UITheme::getCoverThumbPath(std::string coverBmpPath,
                                       int coverHeight) {
  // Mirror the device's thumb path convention so cached files line up if/when
  // we wire actual cover rendering.
  return coverBmpPath + ".thumb" + std::to_string(coverHeight) + ".bmp";
}

UIIcon UITheme::getFileIcon(const std::string& /*filename*/) {
  return static_cast<UIIcon>(0);
}

int UITheme::getStatusBarHeight() { return 32; }
int UITheme::getProgressBarHeight() {
  return LyraMetrics::values.progressBarHeight;
}
