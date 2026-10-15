#ifndef COUNTER4BIT_SC_STIMULUS_H
#define COUNTER4BIT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(counter4bit_sc_stimulus) {
    sc_core::sc_out<bool> reset{"reset"}, clk{"clk"};
    sc_core::sc_out<sc_dt::sc_logic> load{"load"}, enable{"enable"};
    sc_core::sc_in<sc_dt::sc_lv<4>> count_out{"count_out"}, count{"count"};
    unsigned checks = 0, failures = 0;
    unsigned expected_out = 8, expected_count = 0;
    bool count_known = false, out_known = false;
    std::ofstream csv;

    static unsigned next_value(unsigned x) {
        static const unsigned sequence[] = {8, 7, 11, 4, 9, 2, 5, 12, 6, 3, 15, 1, 14, 13};
        for (unsigned i = 0; i < sizeof(sequence) / sizeof(sequence[0]); ++i) {
            if (sequence[i] == x) return sequence[(i + 1) % (sizeof(sequence) / sizeof(sequence[0]))];
        }
        return sequence[0];
    }
    void check(const std::string& name) {
        const auto got_out = count_out.read(), got_count = count.read();
        const sc_dt::sc_lv<4> exp_out = out_known
            ? sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(expected_out)) : sc_dt::sc_lv<4>("XXXX");
        const sc_dt::sc_lv<4> exp_count = count_known ? sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(expected_count)) : sc_dt::sc_lv<4>("XXXX");
        const bool pass = got_out == exp_out && got_count == exp_count;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        const std::string eo = exp_out.to_string(), ec = exp_count.to_string(), ao = got_out.to_string(), ac = got_count.to_string();
        csv << t << ',' << name << ',' << reset.read() << ',' << load.read().to_char() << ',' << enable.read().to_char() << ','
            << eo << ',' << ao << ',' << ec << ',' << ac << ',' << status << '\n'; csv.flush();
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name << " reset=" << reset.read()
                  << " load=" << load.read().to_char() << " enable=" << enable.read().to_char() << " expected_count_out=" << eo
                  << " actual_count_out=" << ao << " expected_count=" << ec << " actual_count=" << ac << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                             << " expected_count_out=" << eo << " actual_count_out=" << ao
                             << " expected_count=" << ec << " actual_count=" << ac << '\n';
    }
    void edge(const char* name, bool l, bool e) {
        load.write(l ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        enable.write(e ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        clk.write(true); wait(1, sc_core::SC_NS);
        const unsigned old_out = expected_out, old_count = expected_count;
        if (reset.read()) { expected_out = 8; out_known = true; }
        else expected_out = old_count;
        if (l && e) { expected_count = next_value(old_out); count_known = true; }
        else if (!l) { expected_count = 8; count_known = true; }
        else if (!e) expected_out = old_count;
        check(name);
        clk.write(false); wait(1, sc_core::SC_NS);
    }
    void run() {
        csv.open("counter4bit.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create counter4bit.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,reset,load,enable,expected_count_out,actual_count_out,expected_count,actual_count,status\n";
        clk.write(false); reset.write(false); load.write(sc_dt::SC_LOGIC_0); enable.write(sc_dt::SC_LOGIC_0); wait(1, sc_core::SC_NS);
        check("uninitialized_registers");
        reset.write(true); wait(1, sc_core::SC_NS); expected_out = 8; out_known = true; check("asynchronous_reset_asserted");
        edge("clock_while_reset_load_low_initializes_count", false, false);
        reset.write(false); wait(1, sc_core::SC_NS);
        for (unsigned i = 0; i < 14; ++i) edge("enabled_sequence_step", true, true);
        edge("load_low_resets_internal_count", false, true);
        edge("load_low_latency_visible_on_next_edge", false, false);
        edge("load_enable_sequence_restart", true, true);
        edge("enable_low_holds_count_and_tracks_count_out", true, false);
        load.write(sc_dt::SC_LOGIC_X); enable.write(sc_dt::SC_LOGIC_1); clk.write(true); wait(1, sc_core::SC_NS);
        expected_out = expected_count; check("unknown_load_holds_internal_count");
        clk.write(false); wait(1, sc_core::SC_NS);
        reset.write(true); wait(1, sc_core::SC_NS); expected_out = 8; out_known = true; check("asynchronous_reset_between_clock_edges");
        reset.write(false); wait(1, sc_core::SC_NS); check("release_reset_without_clock");
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=counter4bit.csv vcd=counter4bit_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(counter4bit_sc_stimulus) { SC_THREAD(run); }
};

#endif
