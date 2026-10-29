/**
 * AnyX360: counters the host reads for its performance log line and overlay
 * (draws, resolves, render target transfers, GPU time from timestamp queries,
 * the command processor's waits for the GPU, and the caches' memory). Updated
 * by the GPU's threads, read by any thread.
 */
#ifndef XENIA_GPU_ANYX360_COUNTERS_H_
#define XENIA_GPU_ANYX360_COUNTERS_H_

#include <atomic>
#include <cstdint>

namespace xe {
namespace gpu {

struct AnyX360Counters {
  std::atomic<uint64_t> draws{0};        // IssueDraw calls
  std::atomic<uint64_t> copies{0};       // resolves (IssueCopy)
  std::atomic<uint64_t> transfers{0};    // render target ownership transfers
  std::atomic<uint64_t> submissions{0};  // ExecuteCommandLists of guest work
  // GPU time of the guest's command lists (timestamp queries), nanoseconds;
  // 0 if the queue has no timestamps.
  std::atomic<uint64_t> gpu_busy_ns{0};
  std::atomic<uint64_t> gpu_timed_submissions{0};
  // Time the command processor thread spent waiting for the GPU to finish
  // earlier submissions (the frame queue full, or a readback), nanoseconds.
  std::atomic<uint64_t> gpu_wait_ns{0};
  // Time the command processor thread spent executing the ring buffer (from
  // the first new command to the end of the ones written), and the part of it
  // spent in WAIT_REG_MEM waiting for memory or a register, nanoseconds. Its
  // work is cp_busy_ns - gpu_wait_ns - cp_wait_mem_ns; the rest of its time it
  // spins or sleeps waiting for commands.
  std::atomic<uint64_t> cp_busy_ns{0};
  std::atomic<uint64_t> cp_wait_mem_ns{0};
  // Memory: the texture cache's host memory now, the render targets created
  // (they live until the cache is cleared at shutdown) and their size, and the
  // pipelines created and the ones whose creation failed.
  std::atomic<uint64_t> texture_bytes{0};
  std::atomic<uint64_t> render_targets{0};
  std::atomic<uint64_t> render_target_bytes{0};
  std::atomic<uint64_t> pipelines{0};
  std::atomic<uint64_t> pipeline_failures{0};
  // Occlusion queries: EVENT_WRITE_ZPD reports, the ones that retired with no
  // passing samples, and (fast modes) the guesses written at event time that
  // the real result contradicted (guessed visible but hidden, and the other
  // way round); draws carrying a VIZ token; draws skipped while their
  // pipeline was being created in the background.
  std::atomic<uint64_t> zpd_reports{0};
  std::atomic<uint64_t> zpd_zero{0};
  std::atomic<uint64_t> zpd_guessed_visible_was_hidden{0};
  std::atomic<uint64_t> zpd_guessed_hidden_was_visible{0};
  std::atomic<uint64_t> viz_draws{0};
  std::atomic<uint64_t> draws_skipped_pipeline{0};
};

AnyX360Counters& GetAnyX360Counters();

}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_ANYX360_COUNTERS_H_
