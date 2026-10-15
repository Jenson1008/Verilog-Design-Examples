#include <systemc>
#include "DUT/cntr_3ud_sc.h"
#include "stimulus/cntr_3ud_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> up_down;
    sc_core::sc_signal<bool> clk, reset;
    sc_core::sc_signal<sc_dt::sc_lv<3>> bin_count;
    cntr_3ud dut("dut");
    cntr_3ud_sc_stimulus test("stimulus");
    dut.up_down(up_down); dut.clk(clk); dut.reset(reset); dut.bin_count(bin_count);
    test.up_down(up_down); test.clk(clk); test.reset(reset); test.bin_count(bin_count);

    auto* vcd = sc_core::sc_create_vcd_trace_file("cntr_3ud_sc_wave");
    if (!vcd) {
        std::cerr << "ERROR: cannot create VCD\n";
        return 2;
    }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, up_down, "up_down");
    sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, reset, "reset");
    sc_core::sc_trace(vcd, bin_count, "bin_count");
    sc_core::sc_trace(vcd, dut.count_state, "dut.count_state");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
