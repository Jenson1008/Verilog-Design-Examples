#ifndef NO_1S_SC_STIMULUS_H
#define NO_1S_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(no_1s_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<16>> i_a{"i_a"};
    sc_core::sc_out<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_lv<4>> no_ones{"no_ones"};
    unsigned checks = 0, failures = 0, index = 0, expected_count = 0;
    std::ofstream csv;
    bool expected_known = false;

    void check(const std::string& name) {
        const auto actual = no_ones.read();
        sc_dt::sc_lv<4> expected;
        if (expected_known) expected = sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(expected_count));
        else expected = "XXXX";
        const bool pass = actual == expected;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const std::string exp = expected.to_string(), got = actual.to_string(), bits = i_a.read().to_string();
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << t << ',' << name << ',' << bits << ',' << reset.read() << ',' << index << ',' << exp << ',' << got << ',' << status << '\n'; csv.flush();
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name << " i_a=" << bits << " reset=" << reset.read()
                  << " bit_index=" << index << " expected_no_ones=" << exp << " actual_no_ones=" << got << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name << " i_a=" << bits
                             << " expected_no_ones=" << exp << " actual_no_ones=" << got << '\n';
    }
    void tick(const char* name, bool rst) {
        reset.write(rst); wait(1, sc_core::SC_NS);
        if (rst) { expected_known = true; expected_count = 0; }
        else if (index < 16) {
            if (!expected_known) { expected_known = false; }
            else if (i_a.read()[index] == sc_dt::SC_LOGIC_1) ++expected_count;
            else if (i_a.read()[index] != sc_dt::SC_LOGIC_0) expected_known = false;
        }
        clk.write(true); wait(1, sc_core::SC_NS);
        ++index;
        check(name);
        clk.write(false); wait(1, sc_core::SC_NS);
    }
    void run() {
        csv.open("no_1s.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create no_1s.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,i_a,reset,bit_index,expected_no_ones,actual_no_ones,status\n";
        clk.write(false); reset.write(false); i_a.write(sc_dt::sc_lv<16>("1010010101011010")); wait(1, sc_core::SC_NS);
        check("uninitialized_output_before_first_edge");
        tick("reset_at_initial_bit_0", true);
        for (unsigned bit = 1; bit < 16; ++bit) {
            if (bit == 15) {
                sc_dt::sc_lv<16> with_unknown = i_a.read();
                with_unknown[15] = sc_dt::SC_LOGIC_X;
                i_a.write(with_unknown);
            }
            tick(bit == 15 ? "unknown_sample_propagates_to_accumulator" : "consume_indexed_input_bit", false);
        }
        tick("hold_after_integer_reaches_16", false);
        tick("reset_clears_count_but_does_not_reset_integer", true);
        tick("post_reset_hold_integer_still_16", false);
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " bit_indices_tested=0..15 csv=no_1s.csv vcd=no_1s_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(no_1s_sc_stimulus) { SC_THREAD(run); }
};

#endif
