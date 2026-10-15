#include <systemc>
#include "DUT/moore_nonol_sc.h"
#include "stimulus/moore_nonol_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> input_signal, clock_signal, reset_signal, output_signal;

    moore_nonol dut("dut");
    moore_nonol_sc_stimulus test("stimulus");
    dut.in(input_signal);
    dut.clk(clock_signal);
    dut.reset(reset_signal);
    dut.out(output_signal);
    test.in(input_signal);
    test.clk(clock_signal);
    test.reset(reset_signal);
    test.out(output_signal);

    auto* vcd = sc_core::sc_create_vcd_trace_file("moore_nonol_sc_wave");
    if (!vcd) {
        std::cerr << "ERROR: cannot create VCD\n";
        return 2;
    }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, input_signal, "in");
    sc_core::sc_trace(vcd, clock_signal, "clk");
    sc_core::sc_trace(vcd, reset_signal, "reset");
    sc_core::sc_trace(vcd, output_signal, "out");
    sc_core::sc_trace(vcd, dut.present_state, "dut.present_state");
    sc_core::sc_trace(vcd, dut.next_state, "dut.next_state");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(vcd);
    return test.failures ? 1 : 0;
}
