#include <systemc>
#include "DUT/mealy_nonol_sc.h"
#include "stimulus/mealy_nonol_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> in, out;
    sc_core::sc_signal<bool> clk, reset;
    mealy_nonol dut("dut");
    mealy_nonol_sc_stimulus test("stimulus");
    dut.in(in); dut.clk(clk); dut.reset(reset); dut.out(out);
    test.in(in); test.clk(clk); test.reset(reset); test.out(out);
    auto* vcd = sc_core::sc_create_vcd_trace_file("mealy_nonol_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, in, "in"); sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, reset, "reset"); sc_core::sc_trace(vcd, out, "out");
    sc_core::sc_trace(vcd, dut.state, "dut.state");
    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
