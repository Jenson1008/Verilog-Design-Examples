#include <systemc>
#include <iostream>
#include "DUT/mux_condt_sc.h"
#include "stimulus/mux_condt_sc_stimulus.h"
int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<2>> a("in0"),b("in1"),c("in2"),d("in3"),sel("select"),out("out");
    mux_condt dut("dut");mux_condt_sc_stimulus stim("stimulus");
    dut.in0(a);dut.in1(b);dut.in2(c);dut.in3(d);dut.select(sel);dut.out(out);
    stim.in0(a);stim.in1(b);stim.in2(c);stim.in3(d);stim.select(sel);stim.out(out);
    auto*vcd=sc_core::sc_create_vcd_trace_file("mux_condt_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,a,"in0");sc_core::sc_trace(vcd,b,"in1");sc_core::sc_trace(vcd,c,"in2");sc_core::sc_trace(vcd,d,"in3");sc_core::sc_trace(vcd,sel,"select");sc_core::sc_trace(vcd,out,"out");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);return stim.failures==0?0:1;
}
