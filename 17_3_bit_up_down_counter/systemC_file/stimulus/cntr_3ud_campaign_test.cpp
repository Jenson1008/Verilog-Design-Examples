#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>

#include <systemc>

#include "DUT/cntr_3ud.h"

namespace {

struct Vector {
    bool clk;
    bool reset;
    bool up_down;
    unsigned int expected;
};

constexpr std::array<Vector, 20> vectors{{
    {true, true, false, 0},
    {true, false, false, 1},
    {true, false, false, 2},
    {true, false, false, 3},
    {true, false, false, 4},
    {true, false, false, 5},
    {true, false, false, 6},
    {true, false, false, 7},
    {true, false, false, 0},
    {true, false, true, 7},
    {true, false, true, 6},
    {true, false, true, 5},
    {true, false, true, 4},
    {true, false, true, 3},
    {true, false, true, 2},
    {true, false, true, 1},
    {true, false, true, 0},
    {true, true, true, 0},
    {true, false, true, 7},
    {true, false, false, 0},
}};

}

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);

    sc_core::sc_signal<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_signal<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_signal<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_signal<sc_dt::sc_lv<3>> bin_count{"bin_count"};

    cntr_3ud dut("dut");
    dut.up_down(up_down);
    dut.clk(clk);
    dut.reset(reset);
    dut.bin_count(bin_count);

    std::ofstream log("cntr_3ud_campaign.csv", std::ios::out | std::ios::trunc);
    if (!log) {
        std::cerr << "Unable to open cntr_3ud_campaign.csv for writing.\n";
        return 1;
    }

    auto* trace_file = sc_core::sc_create_vcd_trace_file("cntr_3ud_campaign_wave");
    if (trace_file == nullptr) {
        std::cerr << "Unable to create cntr_3ud_campaign_wave.vcd.\n";
        return 1;
    }
    trace_file->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace_file, clk, "clk");
    sc_core::sc_trace(trace_file, reset, "reset");
    sc_core::sc_trace(trace_file, up_down, "up_down");
    sc_core::sc_trace(trace_file, bin_count, "bin_count");

    log << "time_ns,cycle,clk,reset,up_down,expected,actual,status\n";
    clk.write(sc_dt::SC_LOGIC_0);
    reset.write(sc_dt::SC_LOGIC_0);
    up_down.write(sc_dt::SC_LOGIC_0);
    sc_core::sc_start(1, sc_core::SC_NS);

    std::size_t failures = 0;
    for (std::size_t cycle = 0; cycle < vectors.size(); ++cycle) {
        const auto& vector = vectors[cycle];
        reset.write(vector.reset ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        up_down.write(vector.up_down ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_0);
        sc_core::sc_start(1, sc_core::SC_NS);

        clk.write(vector.clk ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        sc_core::sc_start(1, sc_core::SC_NS);

        const auto actual = bin_count.read();
        const int actual_value = actual.is_01() ? static_cast<int>(actual.to_uint()) : -1;
        const bool passed = actual == sc_dt::sc_lv<3>(vector.expected);
        const auto reset_value = reset.read();
        const auto direction = up_down.read();
        const auto clock_value = clk.read();

        log << sc_core::sc_time_stamp().to_seconds() * 1.0e9 << ','
            << cycle << ','
            << (clock_value == sc_dt::SC_LOGIC_1 ? 1 : 0) << ','
            << (reset_value == sc_dt::SC_LOGIC_1 ? 1 : 0) << ','
            << (direction == sc_dt::SC_LOGIC_1 ? 1 : 0) << ','
            << vector.expected << ',' << actual_value << ','
            << (passed ? "PASS" : "FAIL") << '\n';

        if (!passed) {
            ++failures;
            std::cerr << "Counter check failed at " << sc_core::sc_time_stamp()
                      << " cycle=" << cycle << " reset=" << reset_value
                      << " up_down=" << direction << " expected="
                      << vector.expected << " actual=" << actual << ".\n";
        }

        clk.write(sc_dt::SC_LOGIC_0);
        sc_core::sc_start(1, sc_core::SC_NS);
    }

    log << "# SUMMARY: " << (failures == 0 ? "PASS" : "FAIL")
        << " checks=" << vectors.size() << " failures=" << failures << '\n';
    log.close();
    sc_core::sc_close_vcd_trace_file(trace_file);
    if (!log) {
        std::cerr << "Failed while writing cntr_3ud_campaign.csv.\n";
        return 1;
    }

    return failures == 0 ? 0 : 1;
}
