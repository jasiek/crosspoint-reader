#pragma once
// SDL key → simulator input mapping. Two flavours:
//   - buttonForKey:    keys that emulate physical device buttons. The result
//                      is a HalGPIO::BTN_* index ready to feed into
//                      HalGPIO::simSetButton, so the rest of the firmware
//                      stack (MappedInputManager, activities) can use the
//                      device-canonical wasPressed/wasReleased API.
//   - hostActionForKey: keys that are sim-only (orientation cycle, screenshot,
//                       quit). These bypass HalGPIO entirely.
//
// Kept in a single header so the keymap is easy to audit/tweak.

#include <SDL2/SDL.h>

#include <cstdint>

#include "HalGPIO.h"

namespace sim {

// Default keyboard layout (mirrors the labels we'd print on the front panel
// for a desk reviewer): arrows = navigation, X/Enter = confirm, Z = back,
// [/] = page navigation, P = power.
inline int8_t buttonForKey(SDL_Keycode k) {
  switch (k) {
    case SDLK_UP:           return HalGPIO::BTN_UP;
    case SDLK_DOWN:         return HalGPIO::BTN_DOWN;
    case SDLK_LEFT:         return HalGPIO::BTN_LEFT;
    case SDLK_RIGHT:        return HalGPIO::BTN_RIGHT;
    case SDLK_z:            return HalGPIO::BTN_BACK;
    case SDLK_x:
    case SDLK_RETURN:       return HalGPIO::BTN_CONFIRM;
    case SDLK_LEFTBRACKET:  return HalGPIO::BTN_UP;     // page back  → side
    case SDLK_RIGHTBRACKET: return HalGPIO::BTN_DOWN;   // page fwd   → side
    case SDLK_p:            return HalGPIO::BTN_POWER;
    default:                return -1;
  }
}

enum class HostAction : uint8_t {
  None,
  Quit,
  CycleOrientation,
  Screenshot,
};

inline HostAction hostActionForKey(SDL_Keycode k) {
  switch (k) {
    case SDLK_ESCAPE: return HostAction::Quit;
    case SDLK_o:      return HostAction::CycleOrientation;
    case SDLK_F12:    return HostAction::Screenshot;
    default:          return HostAction::None;
  }
}

}  // namespace sim
