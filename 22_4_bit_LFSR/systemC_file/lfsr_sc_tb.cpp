#include "DUT/lfsr_sc.h"
#include "stimulus/lfsr_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_logic> clk("clk"), reset("reset");
    sc_signal<sc_dt::sc_lv<4>> data_out("data_out");
    lfsr dut("dut"); dut.clk(clk); dut.reset(reset); dut.data_out(data_out);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,reset,expected,actual,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, clk, "clk"); sc_trace(vcd, reset, "reset"); sc_trace(vcd, data_out, "data_out");
    unsigned checks = 0, failures = 0, reference = 0;
    auto cycle = [&](bool rst, const std::string& name) {
        reset.write(rst ? SC_LOGIC_1 : SC_LOGIC_0);
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        reference = lfsr_oracle_next(reference, rst);
        const sc_dt::sc_lv<4> expected(reference);
        const auto actual = data_out.read();
        const bool pass = actual == expected;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\"," << rst << ",\"" << expected
            << "\",\"" << actual << "\"," << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " reset=" << rst
                  << " expected_data_out=" << expected << " actual_data_out=" << actual
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " reset=" << rst << " expected_data_out=" << expected
                             << " actual_data_out=" << actual << '\n';
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    reset.write(SC_LOGIC_1); clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    run_lfsr_stimulus(cycle);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures
              << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
