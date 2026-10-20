/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_IMGUI_DRAWER_H_
#define XENIA_UI_IMGUI_DRAWER_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "third_party/imgui/imgui.h"
#include "xenia/hid/input_system.h"
#include "xenia/ui/immediate_drawer.h"
#include "xenia/ui/presenter.h"
#include "xenia/ui/window.h"
#include "xenia/ui/window_listener.h"

struct ImDrawData;
struct ImGuiContext;
struct ImGuiIO;
enum ImGuiKey : int;

namespace xe {
namespace ui {

class ImGuiDialog;
class ImGuiNotification;
class Window;

using IconsData = std::map<uint32_t, std::span<const uint8_t>>;

constexpr ImVec2 default_image_icon_size = ImVec2(64.f, 64.f);

class ImGuiDrawer : public WindowInputListener, public UIDrawer {
 public:
  ImGuiDrawer(Window* window, size_t z_order);
  ~ImGuiDrawer();

  ImGuiIO& GetIO();

  void AddDialog(ImGuiDialog* dialog);
  void RemoveDialog(ImGuiDialog* dialog);

  void AddNotification(ImGuiNotification* notification);
  void RemoveNotification(ImGuiNotification* notification);

  // SetPresenter may be called from the destructor.
  void SetPresenter(Presenter* new_presenter);
  void SetImmediateDrawer(ImmediateDrawer* new_immediate_drawer);
  void SetPresenterAndImmediateDrawer(Presenter* new_presenter,
                                      ImmediateDrawer* new_immediate_drawer) {
    SetPresenter(new_presenter);
    SetImmediateDrawer(new_immediate_drawer);
  }

  void Draw(UIDrawContext& ui_draw_context) override;

  void ClearDialogs();
  void EnableNotifications(bool enable) { are_notifications_enabled_ = enable; }
  // AnyX360: whether dialogs repaint (and poll input) continuously, at the UI
  // frame rate. Off, the UI is drawn only with new guest frames, so a passive
  // overlay doesn't make the UI thread repaint at the monitor refresh rate.
  void SetContinuousRepaint(bool enable) { continuous_repaint_ = enable; }
  // AnyX360: asks for a paint if nothing has been drawn for `idle_seconds`
  // (UI thread). Without continuous repaint the guest's frames draw the UI;
  // input and a slow timer use this so dialogs still respond while the guest
  // presents nothing (loading), without adding paints between guest frames:
  // repaints between vsynced guest presents fill the swap chain's queue and
  // slow the guest down.
  void RequestPaintIfIdle(double idle_seconds);
  // AnyX360: sizes in physical pixels: the display size and the mouse aren't converted to 96-DPI
  // units, and the caller makes its fonts and sizes for the DPI (GetDpiScale), so text is drawn at
  // the screen's own resolution instead of magnified.
  void SetPhysicalPixels(bool enable) { physical_pixels_ = enable; }
  float GetDpiScale() const;
  // AnyX360: the font atlas changed (other fonts or sizes): its texture is made again. UI thread,
  // outside Draw.
  void RebuildFontTexture();

  std::unique_ptr<ImmediateTexture> LoadImGuiIcon(
      std::span<const uint8_t> data);
  std::map<uint32_t, std::unique_ptr<ImmediateTexture>> LoadIcons(
      IconsData data);

  ImmediateTexture* GetNotificationIcon(uint8_t user_index) {
    if (user_index >= notification_icon_textures_.size()) {
      user_index = 0;
    }
    return notification_icon_textures_.at(user_index).get();
  }

  ImmediateTexture* GetLockedAchievementIcon() {
    return locked_achievement_icon_.get();
  }

  ImFont* GetTitleFont() {
    if (!GetIO().Fonts->Fonts[1]->IsLoaded()) {
      return GetIO().Fonts->Fonts[0];
    }
    return GetIO().Fonts->Fonts[1];
  }

  void LoadInputSystem(hid::InputSystem* input_system);
  void SetGuideButtonAction(std::function<void(uint8_t)> func);

  bool IsAnyDialogOpen() const { return !dialogs_.empty(); };

 protected:
  void OnKeyDown(KeyEvent& e) override;
  void OnKeyUp(KeyEvent& e) override;
  void OnKeyChar(KeyEvent& e) override;
  void OnMouseDown(MouseEvent& e) override;
  void OnMouseMove(MouseEvent& e) override;
  void OnMouseUp(MouseEvent& e) override;
  void OnMouseWheel(MouseEvent& e) override;
  void OnTouchEvent(TouchEvent& e) override;
  // For now, no need for OnDpiChanged because redrawing is done continuously.

 private:
  void Initialize();
  void InitializeFonts(const float font_size);
  bool LoadCustomFont(ImGuiIO& io, ImFontConfig& font_config, float font_size);
  bool LoadWindowsFont(ImGuiIO& io, ImFontConfig& font_config, float font_size);
  bool LoadJapaneseFont(ImGuiIO& io, float font_size);

  void SetupNotificationTextures();
  void SetupFontTexture();

  void RenderDrawLists(ImDrawData* data, UIDrawContext& ui_draw_context);

  void ClearInput();
  void OnKey(KeyEvent& e, bool is_down);
  void UpdateMousePosition(float x, float y);
  void SwitchToPhysicalMouseAndUpdateMousePosition(const MouseEvent& e);

  bool IsDrawingDialogs() const { return dialog_loop_next_index_ != SIZE_MAX; }
  void DetachIfLastWindowRemoved();
  void UpdateGamepads();

  std::optional<ImGuiKey> VirtualKeyToImGuiKey(VirtualKey vkey);

  Window* window_;
  size_t z_order_;

  ImGuiContext* internal_state_ = nullptr;
  hid::InputSystem* input_system_ = nullptr;

  std::function<void(uint8_t)> onGuidePressFunction_;
  // All currently-attached dialogs that get drawn.
  std::vector<ImGuiDialog*> dialogs_;

  // All queued notifications. Notification at index 0 is currently presented
  // one.
  std::vector<ImGuiNotification*> notifications_;
  // Using an index, not an iterator, because after the erasure, the adjustment
  // must be done for the vector element indices that would be in the iterator
  // range that would be invalidated.
  // SIZE_MAX if not currently in the dialog loop.
  size_t dialog_loop_next_index_ = SIZE_MAX;

  Presenter* presenter_ = nullptr;

  ImmediateDrawer* immediate_drawer_ = nullptr;
  // Resources specific to an immediate drawer - must be destroyed before
  // detaching the presenter.
  std::unique_ptr<ImmediateTexture> font_texture_;
  std::unique_ptr<ImmediateTexture> locked_achievement_icon_;

  std::vector<std::unique_ptr<ImmediateTexture>> notification_icon_textures_;

  // If there's an active pointer, the ImGui mouse is controlled by this touch.
  // If it's TouchEvent::kPointerIDNone, the ImGui mouse is controlled by the
  // mouse.
  uint32_t touch_pointer_id_ = TouchEvent::kPointerIDNone;
  // Whether after the next frame (since the mouse up event needs to be handled
  // with the correct mouse position still), the ImGui mouse position should be
  // reset (for instance, after releasing a touch), so it's not hovering over
  // anything.
  bool reset_mouse_position_after_next_frame_ = false;

  double frame_time_tick_frequency_;
  uint64_t last_frame_time_ticks_;

  bool are_notifications_enabled_ = true;
  bool continuous_repaint_ = true;
  bool physical_pixels_ = false;

  // AnyX360: input without continuous repaint asks for a paint when idle.
  void OnInputWithoutContinuousRepaint();
};

}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_IMGUI_DRAWER_H_
