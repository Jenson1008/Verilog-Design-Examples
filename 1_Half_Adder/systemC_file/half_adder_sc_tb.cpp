#include <systemc>
#include <iostream>
#include "DUT/half_adder_sc.h"
#include "stimulus/half_adder_sc_stimulus.h"

int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic>a("i_a"),b("i_b"),sum("o_sum"),carry("o_carry");
    half_adder dut("dut");half_adder_sc_stimulus stim("stimulus");
    dut.i_a(a);dut.i_b(b);dut.o_sum(sum);dut.o_carry(carry);
    stim.i_a(a);stim.i_b(b);stim.o_sum(sum);stim.o_carry(carry);
    auto*vcd=sc_core::sc_create_vcd_trace_file("half_adder_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}
    vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,a,"i_a");sc_core::sc_trace(vcd,b,"i_b");
    sc_core::sc_trace(vcd,sum,"o_sum");sc_core::sc_trace(vcd,carry,"o_carry");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);
    return stim.failures==0?0:1;
}
