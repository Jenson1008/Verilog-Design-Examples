#include <systemc>
#include <iostream>
#include "DUT/mux_always_sc.h"
#include "stimulus/mux_always_sc_stimulus.h"
int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> in0("in0"),in1("in1"),sel("select"),out("out");
    mux_always dut("dut");mux_always_sc_stimulus stim("stimulus");
    dut.in0(in0);dut.in1(in1);dut.select(sel);dut.out(out);
    stim.in0(in0);stim.in1(in1);stim.select(sel);stim.out(out);
    auto*vcd=sc_core::sc_create_vcd_trace_file("mux_always_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}
    vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,in0,"in0");sc_core::sc_trace(vcd,in1,"in1");
    sc_core::sc_trace(vcd,sel,"select");sc_core::sc_trace(vcd,out,"out");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);
    return stim.failures==0?0:1;
}
