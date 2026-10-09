#ifndef FULL_HALF_ADD_1BIT_TESTBENCH_H
#define FULL_HALF_ADD_1BIT_TESTBENCH_H

#include <systemc>

#include <iostream>
#include <string>

#include "../DUT/full_half_add_1bit.h"
#include "full_half_add_1bit_stimulus.h"

inline int run_full_half_add_1bit_testbench(
    const std::string& log_path,
    const std::string& vcd_path_without_extension,
    bool include_four_state_cases) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);

    sc_core::sc_signal<sc_dt::sc_logic> sig_i_a{"sig_i_a"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_i_b{"sig_i_b"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_i_cin{"sig_i_cin"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_sum{"sig_o_sum"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_carry{"sig_o_carry"};

    full_half_add_1bit dut{"full_half_add_1bit"};
    full_half_add_1bit_stimulus stimulus{
        "full_half_add_1bit_stimulus",
        log_path,
        vcd_path_without_extension + ".vcd",
        include_four_state_cases};

    dut.i_a(sig_i_a);
    dut.i_b(sig_i_b);
    dut.i_cin(sig_i_cin);
    dut.o_sum(sig_o_sum);
    dut.o_carry(sig_o_carry);
    stimulus.i_a(sig_i_a);
    stimulus.i_b(sig_i_b);
    stimulus.i_cin(sig_i_cin);
    stimulus.o_sum(sig_o_sum);
    stimulus.o_carry(sig_o_carry);

    if (!stimulus.log_ready()) {
        std::cerr << "ERROR: cannot create simulation log: " << log_path << '\n';
        return 2;
    }

    sc_core::sc_trace_file* trace =
        sc_core::sc_create_vcd_trace_file(vcd_path_without_extension.c_str());
    if (trace == nullptr) {
        std::cerr << "ERROR: cannot create VCD trace: "
                  << vcd_path_without_extension << ".vcd\n";
        return 2;
    }

    trace->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace, sig_i_a, "i_a");
    sc_core::sc_trace(trace, sig_i_b, "i_b");
    sc_core::sc_trace(trace, sig_i_cin, "i_cin");
    sc_core::sc_trace(trace, sig_o_sum, "o_sum");
    sc_core::sc_trace(trace, sig_o_carry, "o_carry");
    sc_core::sc_trace(trace, dut.w_sum1, "w_sum1");
    sc_core::sc_trace(trace, dut.w_carry1, "w_carry1");
    sc_core::sc_trace(trace, dut.w_carry2, "w_carry2");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace);

    return stimulus.failed() ? 1 : 0;
}

#endif
