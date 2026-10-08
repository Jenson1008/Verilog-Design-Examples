#include "DUT/full_half_add_1bit_sc.h"
#include "stimulus/full_half_add_1bit_sc_stimulus.h"

int sc_main(int, char**) {
    sc_signal<bool> i_a;
    sc_signal<bool> i_b;
    sc_signal<bool> i_cin;
    sc_signal<bool> o_sum;
    sc_signal<bool> o_carry;

    full_half_add_1bit dut("dut");
    full_half_add_1bit_sc_stimulus stimulus("stimulus");
    dut.i_a(i_a);
    dut.i_b(i_b);
    dut.i_cin(i_cin);
    dut.o_sum(o_sum);
    dut.o_carry(o_carry);
    stimulus.i_a(i_a);
    stimulus.i_b(i_b);
    stimulus.i_cin(i_cin);
    stimulus.o_sum(o_sum);
    stimulus.o_carry(o_carry);

    sc_trace_file* wave = sc_create_vcd_trace_file("full_adder_wave");
    sc_trace(wave, i_a, "i_a");
    sc_trace(wave, i_b, "i_b");
    sc_trace(wave, i_cin, "i_cin");
    sc_trace(wave, o_sum, "o_sum");
    sc_trace(wave, o_carry, "o_carry");

    sc_trace(wave, dut.w_sum1, "w_sum1");
    sc_trace(wave, dut.w_carry1, "w_carry1");
    sc_trace(wave, dut.w_carry2, "w_carry2");

    sc_start();
    sc_close_vcd_trace_file(wave);

    if (stimulus.failures != 0) {
        return 1;
    }
    std::cout << "PASS: all 8 input combinations\n";
    return 0;
}