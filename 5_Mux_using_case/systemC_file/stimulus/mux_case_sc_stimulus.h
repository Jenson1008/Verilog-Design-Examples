#ifndef MUX_CASE_SC_STIMULUS_H
#define MUX_CASE_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(mux_case_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<2>> in0{"in0"}, in1{"in1"}, in2{"in2"}, in3{"in3"};
    sc_core::sc_out<sc_dt::sc_lv<2>> select{"select"};
    sc_core::sc_in<sc_dt::sc_lv<2>> out{"out"};
    unsigned checks = 0, failures = 0;
    std::ofstream csv, log;

    static sc_dt::sc_lv<2> bits2(unsigned v) {
        sc_dt::sc_lv<2> r;
        for (int i = 0; i < 2; ++i) r[i] = ((v >> i) & 1U) ? SC_LOGIC_1 : SC_LOGIC_0;
        return r;
    }
    void record(const char* name, const sc_dt::sc_lv<2>& expected,
                unsigned a, unsigned b, unsigned c, unsigned d, unsigned s) {
        const auto actual = out.read();
        const bool pass = actual == expected;
        ++checks; if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << t << ',' << name << ',' << bits2(a) << ',' << bits2(b) << ','
            << bits2(c) << ',' << bits2(d) << ',' << bits2(s) << ','
            << expected << ',' << actual << ',' << status << '\n';
        log << t << ',' << name << ',' << bits2(a) << ',' << bits2(b) << ','
            << bits2(c) << ',' << bits2(d) << ',' << bits2(s) << ','
            << expected << ',' << actual << ',' << status << '\n';
        std::cout << "t=" << sc_core::sc_time_stamp() << " mux_case"
                  << " in0=" << bits2(a) << " in1=" << bits2(b)
                  << " in2=" << bits2(c) << " in3=" << bits2(d)
                  << " select=" << bits2(s) << " expected_out=" << expected
                  << " actual_out=" << actual << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp()
                             << " mux_case select=" << bits2(s)
                             << " expected_out=" << expected << " actual_out=" << actual << '\n';
    }
    void run() {
        csv.open("mux_case.csv", std::ios::trunc); log.open("simulation.log", std::ios::trunc);
        if (!csv || !log) { std::cerr << "ERROR: cannot create result logs\n"; failures=1; sc_core::sc_stop(); return; }
        const char* header = "time_ns,test_name,in0,in1,in2,in3,select,expected_out,actual_out,status\n";
        csv << header; log << header;
        for (unsigned a=0; a<4; ++a) for (unsigned b=0; b<4; ++b)
        for (unsigned c=0; c<4; ++c) for (unsigned d=0; d<4; ++d)
        for (unsigned s=0; s<4; ++s) {
            in0.write(bits2(a)); in1.write(bits2(b)); in2.write(bits2(c)); in3.write(bits2(d));
            select.write(bits2(s)); wait(1, sc_core::SC_NS);
            const unsigned values[] = {a,b,c,d};
            record("exhaustive", bits2(values[s]), a,b,c,d,s);
        }
        // RTL sensitivity is select-only: a data change by itself must hold the prior output.
        in0.write(bits2(0)); select.write(bits2(0)); wait(1, sc_core::SC_NS);
        const auto held = out.read();
        in0.write(bits2(3)); wait(1, sc_core::SC_NS);
        record("data_only_hold", held, 3,0,0,0,0);
        csv.flush(); log << "SUMMARY checks=" << checks << " failures=" << failures
                         << " csv=mux_case.csv vcd=mux_case_wave.vcd\n"; log.flush();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures
                  << " csv=mux_case.csv vcd=mux_case_wave.vcd\n";
        if (!csv || !log) { std::cerr << "ERROR: log write failed\n"; ++failures; }
        sc_core::sc_stop();
    }
    SC_CTOR(mux_case_sc_stimulus) { SC_THREAD(run); }
};

#endif
