#include <systemc>
#include <iostream>
#include "DUT/full_half_add_1bit_sc.h"
#include "stimulus/full_half_add_1bit_sc_stimulus.h"

int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic>a("i_a"),b("i_b"),cin("i_cin"),sum("o_sum"),carry("o_carry");
    sc_core::sc_signal<sc_dt::sc_logic>ha("h_a"),hb("h_b"),hs("h_sum"),hc("h_carry");
    full_half_add_1bit dut("dut");half_adder leaf("leaf_half_adder");
    full_half_add_1bit_sc_stimulus stim("stimulus");
    dut.i_a(a);dut.i_b(b);dut.i_cin(cin);dut.o_sum(sum);dut.o_carry(carry);
    leaf.h_a(ha);leaf.h_b(hb);leaf.h_sum(hs);leaf.h_carry(hc);
    stim.i_a(a);stim.i_b(b);stim.i_cin(cin);stim.o_sum(sum);stim.o_carry(carry);
    stim.h_a(ha);stim.h_b(hb);stim.h_sum(hs);stim.h_carry(hc);
    auto*vcd=sc_core::sc_create_vcd_trace_file("full_half_add_1bit_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}
    vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,a,"i_a");sc_core::sc_trace(vcd,b,"i_b");
    sc_core::sc_trace(vcd,cin,"i_cin");sc_core::sc_trace(vcd,sum,"o_sum");
    sc_core::sc_trace(vcd,carry,"o_carry");
    sc_core::sc_trace(vcd,ha,"leaf_h_a");sc_core::sc_trace(vcd,hb,"leaf_h_b");
    sc_core::sc_trace(vcd,hs,"leaf_h_sum");sc_core::sc_trace(vcd,hc,"leaf_h_carry");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);
    return stim.failures==0?0:1;
}
