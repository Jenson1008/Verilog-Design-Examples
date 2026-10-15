#ifndef CLKDIVIDER_SC_STIMULUS_H
#define CLKDIVIDER_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(clkdivider_sc_stimulus) {
    sc_core::sc_out<bool> reset{"reset"}, clk{"clk"};
    sc_core::sc_in<sc_dt::sc_logic> out_clk{"out_clk"};
    unsigned checks = 0, failures = 0, count = 0;
    sc_dt::sc_logic expected_out = sc_dt::SC_LOGIC_X;
    bool count_known = false;
    std::ofstream csv;
    void check(const char* name) {
        const sc_dt::sc_logic actual = out_clk.read();
        const bool pass = actual == expected_out;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << t << ',' << name << ',' << reset.read() << ',' << clk.read() << ',' << count
            << ',' << expected_out.to_char() << ',' << actual.to_char() << ',' << status << '\n'; csv.flush();
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name << " reset=" << reset.read()
                  << " clk=" << clk.read() << " model_clk_count=" << count << " expected_out_clk=" << expected_out.to_char()
                  << " actual_out_clk=" << actual.to_char() << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                             << " expected_out_clk=" << expected_out.to_char() << " actual_out_clk=" << actual.to_char() << '\n';
    }
    void edge(const char* name, bool rst) {
        reset.write(rst); clk.write(true); wait(1, sc_core::SC_NS);
        if (rst) { count = 0; count_known = true; expected_out = sc_dt::SC_LOGIC_0; }
        else if (count_known) {
            expected_out = (count & 2U) != 0 ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
            count = (count + 1U) & 3U;
        }
        check(name); clk.write(false); wait(1, sc_core::SC_NS);
    }
    void run() {
        csv.open("clkdivider.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create clkdivider.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,reset,clk,reference_count,expected_out_clk,actual_out_clk,status\n";
        clk.write(false); reset.write(false); wait(1, sc_core::SC_NS); check("uninitialized_output_before_reset");
        reset.write(true); wait(1, sc_core::SC_NS); check("reset_between_edges_is_not_asynchronous");
        edge("reset_on_rising_edge", true);
        for (unsigned i = 0; i < 12; ++i) edge("divide_by_four_edge_sequence", false);
        edge("reassert_reset_on_rising_edge", true);
        edge("first_edge_after_reset", false);
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " rising_edges=15 csv=clkdivider.csv vcd=clkdivider_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(clkdivider_sc_stimulus) { SC_THREAD(run); }
};

#endif
