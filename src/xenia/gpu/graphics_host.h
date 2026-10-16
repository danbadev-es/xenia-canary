/**
 ******************************************************************************
 * AnyX360 integration: lets the GPU run without Xenia's CPU and kernel.      *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_GRAPHICS_HOST_H_
#define XENIA_GPU_GRAPHICS_HOST_H_

#include <cstdint>

namespace xe {
namespace gpu {

// Services the graphics system needs from whatever runs the guest. In AnyX360
// the guest is statically recompiled code with its own kernel, so interrupts
// are delivered by the host on one of its guest threads.
class GraphicsHost {
 public:
  virtual ~GraphicsHost() = default;

  // Runs the guest interrupt callback `callback(source, user_data)` as the
  // kernel's DPC would. May be asynchronous.
  virtual void DispatchInterrupt(uint32_t callback, uint32_t user_data,
                                 uint32_t source, uint32_t cpu) = 0;

  // Title ID of the running executable (shader storage, traces).
  virtual uint32_t GetTitleId() = 0;

  // Display resolution reported to the title.
  virtual uint32_t GetDisplayWidth() { return 1280; }
  virtual uint32_t GetDisplayHeight() { return 720; }

  // Called on the command processor thread when it executes a swap, before
  // the frame is handed to the presenter (frame timing).
  virtual void OnSwap() {}
};

}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_GRAPHICS_HOST_H_
