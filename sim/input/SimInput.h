#pragma once
// SDL keyboard → logical Button mapping for the simulator.
// Mirrors the Button enum from src/MappedInputManager.h so future activity
// code can be ported without API changes; we don't reuse MappedInputManager
// itself yet because it pulls in the full settings/persistence stack.

#include <SDL2/SDL.h>

#include <cstdint>

namespace sim {

enum class Button : uint8_t {
  None = 0,
  Up,
  Down,
  Left,
  Right,
  Back,
  Confirm,
  PageBack,
  PageForward,
  // Sim-only
  CycleOrientation,
  Screenshot,
  Quit,
};

// Translate an SDL key event into a logical button. Reflects the device's
// default keymap (arrow keys = navigation, Z = back, X = confirm, [/] = page
// turn). Returns Button::None for unrecognised keys.
inline Button buttonFromSDL(const SDL_KeyboardEvent& e) {
  switch (e.keysym.sym) {
    case SDLK_UP:        return Button::Up;
    case SDLK_DOWN:      return Button::Down;
    case SDLK_LEFT:      return Button::Left;
    case SDLK_RIGHT:     return Button::Right;
    case SDLK_z:         return Button::Back;
    case SDLK_x:
    case SDLK_RETURN:    return Button::Confirm;
    case SDLK_LEFTBRACKET:  return Button::PageBack;
    case SDLK_RIGHTBRACKET: return Button::PageForward;
    case SDLK_o:         return Button::CycleOrientation;
    case SDLK_F12:       return Button::Screenshot;
    case SDLK_ESCAPE:    return Button::Quit;
    default:             return Button::None;
  }
}

}  // namespace sim
