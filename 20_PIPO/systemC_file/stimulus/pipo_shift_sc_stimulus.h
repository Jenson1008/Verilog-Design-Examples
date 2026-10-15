#ifndef PIPO_SHIFT_SC_STIMULUS_H
#define PIPO_SHIFT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(pipo_shift_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<16>> data_in{"data_in"};
    sc_core::sc_out<sc_dt::sc_logic> load{"load"};
    sc_core::sc_out<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_lv<2>> shift_en{"shift_en"};
    sc_core::sc_in<sc_dt::sc_lv<16>> data_out{"data_out"};
    unsigned checks = 0, failures = 0;
    std::ofstream csv;
    sc_dt::sc_lv<16> temp;
    sc_dt::sc_lv<16> expected;
    bool temp_known = false;

    static sc_dt::sc_lv<16> left_logical(const sc_dt::sc_lv<16>& x) {
        sc_dt::sc_lv<16> y; y[0] = sc_dt::SC_LOGIC_0;
        for (int bit = 1; bit < 16; ++bit) y[bit] = x[bit - 1];
        return y;
    }
    static sc_dt::sc_lv<16> right_logical(const sc_dt::sc_lv<16>& x) {
        sc_dt::sc_lv<16> y; y[15] = sc_dt::SC_LOGIC_0;
        for (int bit = 0; bit < 15; ++bit) y[bit] = x[bit + 1];
        return y;
    }
    static sc_dt::sc_lv<16> right_arithmetic(const sc_dt::sc_lv<16>& x) {
        sc_dt::sc_lv<16> y; y[15] = x[15];
        for (int bit = 14; bit >= 0; --bit) y[bit] = x[bit + 1];
        return y;
    }
    void check(const std::string& name) {
        const auto actual = data_out.read(); const bool pass = actual == expected;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const std::string din = data_in.read().to_string(), mode = shift_en.read().to_string();
        const std::string exp = expected.to_string(), got = actual.to_string();
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << t << ',' << name << ',' << din << ',' << mode << ',' << load.read().to_char() << ',' << reset.read()
            << ',' << exp << ',' << got << ',' << status << '\n'; csv.flush();
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name << " data_in=" << din << " shift_en=" << mode
                  << " load=" << load.read().to_char() << " reset=" << reset.read() << " expected_data_out=" << exp
                  << " actual_data_out=" << got << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                             << " expected_data_out=" << exp << " actual_data_out=" << got << '\n';
    }
    void edge(const std::string& name, const sc_dt::sc_lv<16>& din, bool l, bool rst, unsigned mode) {
        data_in.write(din); load.write(l ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        reset.write(rst); shift_en.write(sc_dt::sc_lv<2>(sc_dt::sc_uint<2>(mode)));
        clk.write(true); wait(1, sc_core::SC_NS);
        if (rst) expected = "0000000000000000";
        else if (l) {
            sc_dt::sc_lv<16> old = temp;
            sc_dt::sc_lv<2> sel{sc_dt::sc_uint<2>(mode)};
            if (sel == sc_dt::sc_lv<2>("00")) expected = left_logical(old);
            else if (sel == sc_dt::sc_lv<2>("01")) expected = right_logical(old);
            else if (sel == sc_dt::sc_lv<2>("10")) expected = left_logical(old);
            else expected = right_arithmetic(old);
            temp = din; temp_known = true;
        }
        check(name);
        clk.write(false); wait(1, sc_core::SC_NS);
    }
    void run() {
        csv.open("pipo_shift.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create pipo_shift.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,data_in,shift_en,load,reset,expected_data_out,actual_data_out,status\n";
        clk.write(false); reset.write(false); load.write(sc_dt::SC_LOGIC_0); data_in.write(sc_dt::sc_lv<16>("0000000000000000"));
        shift_en.write(sc_dt::sc_lv<2>("00")); expected = "XXXXXXXXXXXXXXXX"; wait(1, sc_core::SC_NS);
        check("uninitialized_data_out");
        edge("synchronous_reset", sc_dt::sc_lv<16>("0000000000000000"), false, true, 0);
        edge("first_load_uses_uninitialized_old_temp", sc_dt::sc_lv<16>("1000111000010110"), true, false, 0);
        edge("load_shift_right_uses_previous_temp", sc_dt::sc_lv<16>("0101010101010101"), true, false, 1);
        edge("load_arithmetic_left_on_unsigned_register", sc_dt::sc_lv<16>("1000000000000011"), true, false, 2);
        edge("load_arithmetic_right_explicit_sign_bit", sc_dt::sc_lv<16>("0111111111111110"), true, false, 3);
        edge("load_low_holds_data_out", sc_dt::sc_lv<16>("1111000011110000"), false, false, 0);
        edge("reset_does_not_clear_temp", sc_dt::sc_lv<16>("0000000000000000"), false, true, 0);
        edge("post_reset_load_shifts_preserved_temp", sc_dt::sc_lv<16>("0011001100110011"), true, false, 3);
        data_in.write(sc_dt::sc_lv<16>("1111111100000000"));
        load.write(sc_dt::SC_LOGIC_1); reset.write(false); shift_en.write(sc_dt::sc_lv<2>("0X"));
        clk.write(true); wait(1, sc_core::SC_NS); temp = data_in.read(); temp_known = true;
        check("unknown_shift_selector_holds_output");
        clk.write(false); wait(1, sc_core::SC_NS);
        data_in.write(sc_dt::sc_lv<16>("0000000011111111"));
        load.write(sc_dt::SC_LOGIC_X); shift_en.write(sc_dt::sc_lv<2>("01"));
        clk.write(true); wait(1, sc_core::SC_NS);
        check("unknown_load_is_not_true_and_holds_registers");
        clk.write(false); wait(1, sc_core::SC_NS);
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=pipo_shift.csv vcd=pipo_shift_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(pipo_shift_sc_stimulus) { SC_THREAD(run); }
};

#endif
