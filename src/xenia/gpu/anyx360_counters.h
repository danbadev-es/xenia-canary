/**
 * AnyX360: counters the host reads for its performance log line and overlay
 * (draws, resolves, render target transfers, GPU time from timestamp queries,
 * and the command processor's waits for the GPU). Updated by the command
 * processor thread, read by any thread.
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
};

AnyX360Counters& GetAnyX360Counters();

}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_ANYX360_COUNTERS_H_
