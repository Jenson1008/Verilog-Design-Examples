#ifndef MEALY_OL_SC_STIMULUS_H
#define MEALY_OL_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(mealy_ol_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> in{"in"};
    sc_core::sc_out<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_logic> out{"out"};
    unsigned checks = 0, failures = 0;
    std::ofstream csv;
    unsigned state = 0;
    void check(const std::string& name, sc_dt::sc_logic expected, sc_dt::sc_logic bit) {
        const sc_dt::sc_logic actual = out.read(); const bool pass = actual == expected;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << t << ',' << name << ',' << bit.to_char() << ',' << expected.to_char() << ',' << actual.to_char() << ',' << status << '\n'; csv.flush();
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name
                  << " in=" << bit.to_char() << " expected_out=" << expected.to_char() << " actual_out=" << actual.to_char() << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                             << " in=" << bit.to_char() << " expected_out=" << expected.to_char() << " actual_out=" << actual.to_char() << '\n';
    }
    sc_dt::sc_logic expected_output(bool bit) const { return state == 3 && bit ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;     }
    void advance(bool bit) {
        static const unsigned transitions[4][2] = {{0, 1}, {0, 2}, {3, 2}, {0, 1}};
        state = transitions[state][bit ? 1 : 0];
    }
    void run() {
        csv.open("mealy_ol.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create mealy_ol.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,in,expected_out,actual_out,status\n";
        clk.write(false); in.write(sc_dt::SC_LOGIC_0); reset.write(false); wait(1, sc_core::SC_NS);
        check("uninitialized_state_and_output", sc_dt::SC_LOGIC_X, in.read());
        reset.write(true); wait(1, sc_core::SC_NS); state = 0; check("asynchronous_reset", sc_dt::SC_LOGIC_0, in.read());
        reset.write(false); wait(1, sc_core::SC_NS);
        in.write(sc_dt::SC_LOGIC_X); wait(1, sc_core::SC_NS);
        check("X_input_if_branch_in_S0", sc_dt::SC_LOGIC_0, in.read());
        clk.write(true); wait(1, sc_core::SC_NS); advance(false);
        check("X_input_treated_as_false_on_edge", sc_dt::SC_LOGIC_0, in.read());
        clk.write(false); wait(1, sc_core::SC_NS);
        const char* vectors[] = {"1101101", "11101101101", "0101101101"};
        for (unsigned v = 0; v < 3; ++v) {
            if (v) { reset.write(true); wait(1, sc_core::SC_NS); state = 0; check("reset_between_edges", sc_dt::SC_LOGIC_0, in.read()); reset.write(false); wait(1, sc_core::SC_NS); }
            for (const char* p = vectors[v]; *p; ++p) {
                const bool bit = *p == '1'; const sc_dt::sc_logic input_bit = bit ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
                in.write(input_bit); wait(1, sc_core::SC_NS);
                check("mealy_before_edge", expected_output(bit), input_bit);
                clk.write(true); wait(1, sc_core::SC_NS); advance(bit);
                check("mealy_after_edge", expected_output(bit), input_bit);
                clk.write(false); wait(1, sc_core::SC_NS);
            }
        }
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=mealy_ol.csv vcd=mealy_ol_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(mealy_ol_sc_stimulus) { SC_THREAD(run); }
};

#endif
