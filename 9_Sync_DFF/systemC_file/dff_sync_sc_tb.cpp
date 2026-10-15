#include <systemc>
#include <iostream>
#include "DUT/dff_sync_sc.h"
#include "stimulus/dff_sync_sc_stimulus.h"
int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic>d("i_d"),clk("clk"),reset("reset"),q("o_q"),qb("o_qb");
    dff_sync dut("dut");dff_sync_sc_stimulus stim("stimulus");
    dut.i_d(d);dut.clk(clk);dut.reset(reset);dut.o_q(q);dut.o_qb(qb);
    stim.i_d(d);stim.clk(clk);stim.reset(reset);stim.o_q(q);stim.o_qb(qb);
    auto*vcd=sc_core::sc_create_vcd_trace_file("dff_sync_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,d,"i_d");sc_core::sc_trace(vcd,clk,"clk");sc_core::sc_trace(vcd,reset,"reset");
    sc_core::sc_trace(vcd,q,"o_q");sc_core::sc_trace(vcd,qb,"o_qb");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);return stim.failures==0?0:1;
}
