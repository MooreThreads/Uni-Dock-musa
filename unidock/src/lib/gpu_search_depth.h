#pragma once

// The CPU oracle always uses the complete heuristic Monte Carlo depth.  The
// native MUSA correctness route must not silently replace that depth with the
// GPU throughput preset before it has established numerical equivalence.
constexpr unsigned gpu_heuristic_search_depth(unsigned heuristic) {
    return unsigned(70 * 3 * (50 + heuristic) / 2);
}

constexpr unsigned gpu_search_depth(unsigned heuristic, int requested_cap,
                                    bool require_cpu_oracle_parity) {
    const unsigned full_depth = gpu_heuristic_search_depth(heuristic);
    if (require_cpu_oracle_parity || requested_cap <= 0) return full_depth;
    return requested_cap < static_cast<int>(full_depth) ? unsigned(requested_cap) : full_depth;
}

constexpr bool native_gpu_requires_cpu_oracle_depth() {
#ifdef ENABLE_MUSA
    return true;
#else
    return false;
#endif
}
