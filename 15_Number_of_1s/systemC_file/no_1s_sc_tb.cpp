#include <systemc>
#include "DUT/no_1s_sc.h"
#include "stimulus/no_1s_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<16>> i_a;
    sc_core::sc_signal<bool> clk, reset;
    sc_core::sc_signal<sc_dt::sc_lv<4>> no_ones;
    no_1s dut("dut"); no_1s_sc_stimulus test("stimulus");
    dut.i_a(i_a); dut.clk(clk); dut.reset(reset); dut.no_ones(no_ones);
    test.i_a(i_a); test.clk(clk); test.reset(reset); test.no_ones(no_ones);
    auto* vcd = sc_core::sc_create_vcd_trace_file("no_1s_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, i_a, "i_a"); sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, reset, "reset"); sc_core::sc_trace(vcd, no_ones, "no_ones");
    sc_core::sc_trace(vcd, dut.i, "dut.i");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
