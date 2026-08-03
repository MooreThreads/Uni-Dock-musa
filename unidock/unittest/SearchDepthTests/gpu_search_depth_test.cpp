#include <catch2/catch_test_macros.hpp>

#include "gpu_search_depth.h"

TEST_CASE("native MUSA correctness keeps CPU-oracle Monte Carlo depth") {
    const unsigned heuristic = 171;  // 1IEP: 41 movable atoms + 10 * 13 degrees of freedom

    REQUIRE(gpu_heuristic_search_depth(heuristic) == 23205);
    REQUIRE(native_gpu_requires_cpu_oracle_depth());
    REQUIRE(gpu_search_depth(heuristic, 20, true) == 23205);
}

TEST_CASE("CUDA throughput preset remains bounded by its requested cap") {
    REQUIRE(gpu_search_depth(171, 20, false) == 20);
    REQUIRE(gpu_search_depth(171, 0, false) == 23205);
}
