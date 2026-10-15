#include <systemc>
#include <iostream>
#include "DUT/full_add_4bit_sc.h"
#include "stimulus/full_add_4bit_sc_stimulus.h"

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_lv<4>> a("i_a"), b("i_b"), sum("o_sum");
    sc_core::sc_signal<sc_dt::sc_logic> cin("i_cin"), carry("o_carry");
    sc_core::sc_signal<sc_dt::sc_logic> ha_a("ha_a"),ha_b("ha_b"),ha_sum("ha_sum"),ha_carry("ha_carry");
    sc_core::sc_signal<sc_dt::sc_logic> fa_a("fa_a"),fa_b("fa_b"),fa_cin("fa_cin"),fa_sum("fa_sum"),fa_carry("fa_carry");
    full_add_4bit dut("dut"); full_add_4bit_sc_stimulus stimulus("stimulus");
    half_adder leaf_half("leaf_half");
    full_half_add_1bit leaf_full("leaf_full");
    dut.i_a(a); dut.i_b(b); dut.i_cin(cin); dut.o_sum(sum); dut.o_carry(carry);
    stimulus.i_a(a); stimulus.i_b(b); stimulus.i_cin(cin);
    stimulus.o_sum(sum); stimulus.o_carry(carry);
    leaf_half.h_a(ha_a);leaf_half.h_b(ha_b);leaf_half.h_sum(ha_sum);leaf_half.h_carry(ha_carry);
    leaf_full.i_a(fa_a);leaf_full.i_b(fa_b);leaf_full.i_cin(fa_cin);
    leaf_full.o_sum(fa_sum);leaf_full.o_carry(fa_carry);
    stimulus.ha_a(ha_a);stimulus.ha_b(ha_b);stimulus.ha_sum(ha_sum);stimulus.ha_carry(ha_carry);
    stimulus.fa_a(fa_a);stimulus.fa_b(fa_b);stimulus.fa_cin(fa_cin);
    stimulus.fa_sum(fa_sum);stimulus.fa_carry(fa_carry);
    auto* trace = sc_core::sc_create_vcd_trace_file("full_add_4bit_wave");
    if (!trace) { std::cerr << "ERROR: cannot create VCD\n"; return 1; }
    trace->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace, a, "i_a"); sc_core::sc_trace(trace, b, "i_b");
    sc_core::sc_trace(trace, cin, "i_cin"); sc_core::sc_trace(trace, sum, "o_sum");
    sc_core::sc_trace(trace, carry, "o_carry");
    sc_core::sc_trace(trace,ha_a,"leaf_ha_a");sc_core::sc_trace(trace,ha_b,"leaf_ha_b");
    sc_core::sc_trace(trace,ha_sum,"leaf_ha_sum");sc_core::sc_trace(trace,ha_carry,"leaf_ha_carry");
    sc_core::sc_trace(trace,fa_a,"leaf_fa_a");sc_core::sc_trace(trace,fa_b,"leaf_fa_b");
    sc_core::sc_trace(trace,fa_cin,"leaf_fa_cin");sc_core::sc_trace(trace,fa_sum,"leaf_fa_sum");
    sc_core::sc_trace(trace,fa_carry,"leaf_fa_carry");
    sc_core::sc_start(); sc_core::sc_close_vcd_trace_file(trace);
    return stimulus.failures == 0 ? 0 : 1;
}
