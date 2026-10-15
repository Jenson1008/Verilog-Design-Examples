#include "DUT/univ_shift_sc.h"
#include "stimulus/univ_shift_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_lv<16>> data_in("data_in"), data_out("data_out");
    sc_signal<sc_dt::sc_lv<2>> control("control");
    sc_signal<sc_dt::sc_logic> clk("clk"), reset("reset");
    univ_shift dut("dut");
    dut.data_in(data_in); dut.control(control); dut.clk(clk); dut.reset(reset); dut.data_out(data_out);

    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,input_data,control,expected,actual,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, data_in, "data_in"); sc_trace(vcd, control, "control");
    sc_trace(vcd, clk, "clk"); sc_trace(vcd, reset, "reset"); sc_trace(vcd, data_out, "data_out");

    unsigned checks = 0, failures = 0;
    auto check = [&](const std::string& name, const sc_dt::sc_lv<16>& input,
                     unsigned mode, const sc_dt::sc_lv<16>& expected) {
        data_in.write(input); control.write(mode);
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
        const auto actual = data_out.read();
        const bool pass = actual == expected;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\",\"" << input << "\"," << mode
            << ",\"" << expected << "\",\"" << actual << "\"," << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " data_in=" << input
                  << " control=" << mode << " expected=" << expected
                  << " actual=" << actual << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " data_in=" << input << " control=" << mode
                             << " expected_data_out=" << expected << " actual_data_out=" << actual << '\n';
    };

    clk.write(SC_LOGIC_0); reset.write(SC_LOGIC_1); data_in.write(0); control.write(0);
    sc_start(1, SC_NS);
    // Asserted-high reset is sampled at a clock edge, then deassertion triggers
    // the RTL's negedge-reset event with the active-low datapath condition.
    clk.write(SC_LOGIC_1);
    sc_start(1, SC_NS);
    clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    reset.write(SC_LOGIC_0); sc_start(1, SC_NS);
    run_univ_shift_vectors(check);

    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures
              << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
