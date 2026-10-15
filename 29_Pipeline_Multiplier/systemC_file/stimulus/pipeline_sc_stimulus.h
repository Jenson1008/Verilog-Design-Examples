#ifndef PIPELINE_SC_STIMULUS_H
#define PIPELINE_SC_STIMULUS_H

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

struct PipelineInputVector {
    std::string name;
    std::uint64_t x;
    std::uint64_t y;
};

inline constexpr std::uint64_t PIPELINE_RANDOM_SEED =
    UINT64_C(0x29D1B54A32C0FFEE);

inline std::uint64_t pipeline_xorshift64(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

inline std::vector<PipelineInputVector> pipeline_test_vectors() {
    std::vector<PipelineInputVector> vectors{
        {"one_by_one", UINT64_C(1), UINT64_C(1)},
        {"zero_by_all_ones", UINT64_C(0), UINT64_MAX},
        {"low_32_max_square", UINT64_C(0x00000000FFFFFFFF),
         UINT64_C(0x00000000FFFFFFFF)},
        {"all_ones_by_one", UINT64_MAX, UINT64_C(1)},
        {"high_bit_times_two", UINT64_C(0x8000000000000000),
         UINT64_C(2)},
        {"alternating_patterns", UINT64_C(0xAAAAAAAAAAAAAAAA),
         UINT64_C(0x5555555555555555)},
        {"mixed_hex", UINT64_C(0x0123456789ABCDEF),
         UINT64_C(0xFEDCBA9876543210)},
        {"carry_stress", UINT64_C(0xFFFFFFFF00000001),
         UINT64_C(0x00000001FFFFFFFF)}
    };

    std::uint64_t state = PIPELINE_RANDOM_SEED;
    for (unsigned int index = 0; index < 16; ++index) {
        const std::uint64_t x = pipeline_xorshift64(state);
        const std::uint64_t y = pipeline_xorshift64(state);
        std::ostringstream name;
        name << "xorshift_seed_29_" << std::setw(2) << std::setfill('0')
             << index;
        vectors.push_back({name.str(), x, y});
    }
    return vectors;
}

#endif  // PIPELINE_SC_STIMULUS_H
