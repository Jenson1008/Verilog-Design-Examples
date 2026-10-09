#include <cstdlib>
#include <iostream>

#include <systemc>

#include "DUT/cntr_3ud.h"
#include "stimulus/cntr_3ud_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);

    sc_core::sc_signal<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_signal<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_signal<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_signal<sc_dt::sc_lv<3>> bin_count{"bin_count"};

    cntr_3ud dut("dut");
    cntr_3ud_stimulus stimulus("stimulus");

    dut.up_down(up_down);
    dut.clk(clk);
    dut.reset(reset);
    dut.bin_count(bin_count);

    stimulus.up_down(up_down);
    stimulus.clk(clk);
    stimulus.reset(reset);
    stimulus.bin_count(bin_count);

    auto* trace_file = sc_core::sc_create_vcd_trace_file("cntr_3ud_wave");
    if (trace_file == nullptr) {
        std::cerr << "Unable to create the counter VCD trace file.\n";
        return EXIT_FAILURE;
    }
    trace_file->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace_file, clk, "clk");
    sc_core::sc_trace(trace_file, reset, "reset");
    sc_core::sc_trace(trace_file, up_down, "up_down");
    sc_core::sc_trace(trace_file, bin_count, "bin_count");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace_file);

    return stimulus.result() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
