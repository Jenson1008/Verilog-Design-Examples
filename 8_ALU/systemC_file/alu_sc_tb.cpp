#include <systemc>
#include <iostream>
#include "DUT/alu_sc.h"
#include "stimulus/alu_sc_stimulus.h"
int sc_main(int,char**){
    sc_core::sc_set_time_resolution(1,sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<4>> a("a"),b("b"),out("out");
    sc_core::sc_signal<sc_dt::sc_lv<2>> sel("select");
    sc_core::sc_signal<sc_dt::sc_logic> zero("zero"),carry("carry"),sign("sign"),parity("parity"),overflow("overflow");
    alu dut("dut");alu_sc_stimulus stim("stimulus");
    dut.a(a);dut.b(b);dut.select(sel);dut.out(out);dut.zero(zero);dut.carry(carry);dut.sign(sign);dut.parity(parity);dut.overflow(overflow);
    stim.a(a);stim.b(b);stim.select(sel);stim.out(out);stim.zero(zero);stim.carry(carry);stim.sign(sign);stim.parity(parity);stim.overflow(overflow);
    auto*vcd=sc_core::sc_create_vcd_trace_file("alu_wave");
    if(!vcd){std::cerr<<"ERROR: cannot create VCD\n";return 1;}vcd->set_time_unit(1,sc_core::SC_NS);
    sc_core::sc_trace(vcd,a,"a");sc_core::sc_trace(vcd,b,"b");sc_core::sc_trace(vcd,sel,"select");
    sc_core::sc_trace(vcd,out,"out");sc_core::sc_trace(vcd,zero,"zero");sc_core::sc_trace(vcd,carry,"carry");
    sc_core::sc_trace(vcd,sign,"sign");sc_core::sc_trace(vcd,parity,"parity");sc_core::sc_trace(vcd,overflow,"overflow");
    sc_core::sc_start();sc_core::sc_close_vcd_trace_file(vcd);return stim.failures==0?0:1;
}
