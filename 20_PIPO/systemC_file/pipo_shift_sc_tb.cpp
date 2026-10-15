#include <systemc>
#include "DUT/pipo_shift_sc.h"
#include "stimulus/pipo_shift_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<16>> data_in, data_out;
    sc_core::sc_signal<sc_dt::sc_lv<2>> shift_en;
    sc_core::sc_signal<sc_dt::sc_logic> load;
    sc_core::sc_signal<bool> clk, reset;
    pipo_shift dut("dut"); pipo_shift_sc_stimulus test("stimulus");
    dut.data_in(data_in); dut.load(load); dut.shift_en(shift_en); dut.clk(clk); dut.reset(reset); dut.data_out(data_out);
    test.data_in(data_in); test.load(load); test.shift_en(shift_en); test.clk(clk); test.reset(reset); test.data_out(data_out);
    auto* vcd = sc_core::sc_create_vcd_trace_file("pipo_shift_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, data_in, "data_in"); sc_core::sc_trace(vcd, shift_en, "shift_en");
    sc_core::sc_trace(vcd, load, "load"); sc_core::sc_trace(vcd, clk, "clk");
    sc_core::sc_trace(vcd, reset, "reset"); sc_core::sc_trace(vcd, data_out, "data_out");
    sc_core::sc_trace(vcd, dut.temp_data, "dut.temp_data");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
