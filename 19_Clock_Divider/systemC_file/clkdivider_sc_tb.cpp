#include <systemc>
#include "DUT/clkdivider_sc.h"
#include "stimulus/clkdivider_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<bool> reset, clk;
    sc_core::sc_signal<sc_dt::sc_logic> out_clk;
    clkdivider dut("dut"); clkdivider_sc_stimulus test("stimulus");
    dut.reset(reset); dut.clk(clk); dut.out_clk(out_clk);
    test.reset(reset); test.clk(clk); test.out_clk(out_clk);
    auto* vcd = sc_core::sc_create_vcd_trace_file("clkdivider_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, reset, "reset"); sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, out_clk, "out_clk");
    sc_core::sc_trace(vcd, dut.clk_count, "dut.clk_count");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
