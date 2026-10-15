#include <systemc>
#include "DUT/counter4bit_sc.h"
#include "stimulus/counter4bit_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<bool> reset, clk;
    sc_core::sc_signal<sc_dt::sc_logic> load, enable;
    sc_core::sc_signal<sc_dt::sc_lv<4>> count_out, count;
    counter4bit dut("dut"); counter4bit_sc_stimulus test("stimulus");
    dut.reset(reset); dut.load(load); dut.enable(enable); dut.clk(clk); dut.count_out(count_out); dut.count(count);
    test.reset(reset); test.load(load); test.enable(enable); test.clk(clk); test.count_out(count_out); test.count(count);
    auto* vcd = sc_core::sc_create_vcd_trace_file("counter4bit_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, reset, "reset"); sc_core::sc_trace(vcd, load, "load");
    sc_core::sc_trace(vcd, enable, "enable"); sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, count_out, "count_out"); sc_core::sc_trace(vcd, count, "count");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
