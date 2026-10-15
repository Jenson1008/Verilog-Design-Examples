#include <systemc>
#include "DUT/bin_gry_sc.h"
#include "stimulus/bin_gry_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<8>> binary, gray;
    bin_gry<8> dut("dut"); bin_gry_sc_stimulus test("stimulus");
    dut.binary(binary); dut.gray(gray); test.binary(binary); test.gray(gray);
    auto* vcd = sc_core::sc_create_vcd_trace_file("bin_gry_wave");
    if (!vcd) { std::cerr << "ERROR: cannot create VCD\n"; return 2; }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, binary, "binary"); sc_core::sc_trace(vcd, gray, "gray");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
