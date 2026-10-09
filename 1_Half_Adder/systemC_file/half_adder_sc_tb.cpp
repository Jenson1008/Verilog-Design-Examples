#include "DUT/half_adder_sc.h"
#include "stimulus/half_adder_sc_stimulus.h"

int sc_main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);

    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);

    sc_core::sc_signal<sc_dt::sc_logic> sig_i_a{"sig_i_a"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_i_b{"sig_i_b"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_sum{"sig_o_sum"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_carry{"sig_o_carry"};

    half_adder dut{"half_adder"};
    half_adder_stimulus stimulus{"half_adder_stimulus", "half_adder.csv"};

    dut.i_a(sig_i_a);
    dut.i_b(sig_i_b);
    dut.o_sum(sig_o_sum);
    dut.o_carry(sig_o_carry);
    stimulus.i_a(sig_i_a);
    stimulus.i_b(sig_i_b);
    stimulus.o_sum(sig_o_sum);
    stimulus.o_carry(sig_o_carry);

    sc_core::sc_trace_file* trace =
        sc_core::sc_create_vcd_trace_file("half_adder_wave");
    if (trace == nullptr) {
        std::cerr << "ERROR: cannot create VCD file half_adder_wave.vcd\n";
        return 2;
    }

    trace->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace, sig_i_a, "i_a");
    sc_core::sc_trace(trace, sig_i_b, "i_b");
    sc_core::sc_trace(trace, sig_o_sum, "o_sum");
    sc_core::sc_trace(trace, sig_o_carry, "o_carry");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace);
    return stimulus.failed() ? 1 : 0;
}
