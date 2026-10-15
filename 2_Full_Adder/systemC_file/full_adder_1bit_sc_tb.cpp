#include <systemc>
#include "DUT/full_adder_1bit_sc.h"
#include "stimulus/full_adder_1bit_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> i_a("i_a"), i_b("i_b"), i_cin("i_cin");
    sc_core::sc_signal<sc_dt::sc_logic> o_sum("o_sum"), o_carry("o_carry");
    full_adder_1bit dut("dut");
    full_adder_1bit_sc_stimulus stimulus("stimulus");
    dut.i_a(i_a); dut.i_b(i_b); dut.i_cin(i_cin);
    dut.o_sum(o_sum); dut.o_carry(o_carry);
    stimulus.i_a(i_a); stimulus.i_b(i_b); stimulus.i_cin(i_cin);
    stimulus.o_sum(o_sum); stimulus.o_carry(o_carry);

    sc_core::sc_trace_file* trace =
        sc_core::sc_create_vcd_trace_file("full_adder_1bit_wave");
    if (!trace) {
        std::cerr << "ERROR: cannot create VCD trace\n";
        return 1;
    }
    trace->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace, i_a, "i_a");
    sc_core::sc_trace(trace, i_b, "i_b");
    sc_core::sc_trace(trace, i_cin, "i_cin");
    sc_core::sc_trace(trace, o_sum, "o_sum");
    sc_core::sc_trace(trace, o_carry, "o_carry");
    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace);
    return stimulus.failures == 0 ? 0 : 1;
}
