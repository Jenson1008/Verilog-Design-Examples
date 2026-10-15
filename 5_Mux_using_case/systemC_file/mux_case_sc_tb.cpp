#include <systemc>
#include <iostream>
#include "DUT/mux_case_sc.h"
#include "stimulus/mux_case_sc_stimulus.h"
int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<2>> in0("in0"),in1("in1"),in2("in2"),in3("in3"),sel("select"),out("out");
    mux_case dut("dut"); mux_case_sc_stimulus stimulus("stimulus");
    dut.in0(in0); dut.in1(in1); dut.in2(in2); dut.in3(in3); dut.select(sel); dut.out(out);
    stimulus.in0(in0); stimulus.in1(in1); stimulus.in2(in2); stimulus.in3(in3);
    stimulus.select(sel); stimulus.out(out);
    auto* vcd=sc_core::sc_create_vcd_trace_file("mux_case_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}
    vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,in0,"in0");sc_core::sc_trace(vcd,in1,"in1");
    sc_core::sc_trace(vcd,in2,"in2");sc_core::sc_trace(vcd,in3,"in3");
    sc_core::sc_trace(vcd,sel,"select");sc_core::sc_trace(vcd,out,"out");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);
    return stimulus.failures==0?0:1;
}
