#include "DUT/custdesign_sc.h"
#include "stimulus/custdesign_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_logic> x("x"), clk("clk"), reset("reset"), y("y");
    custdesign dut("dut"); dut.x(x); dut.clk(clk); dut.reset(reset); dut.y(y);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,x,reset,expected_y,actual_y,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, x, "x"); sc_trace(vcd, reset, "reset"); sc_trace(vcd, clk, "clk"); sc_trace(vcd, y, "y");
    sc_trace(vcd, dut.a, "dut.a"); sc_trace(vcd, dut.abar, "dut.abar");
    sc_trace(vcd, dut.b, "dut.b"); sc_trace(vcd, dut.bbar, "dut.bbar");
    CustdesignReference reference;
    unsigned checks = 0, failures = 0;
    auto cycle = [&](bool rst, bool xv, const std::string& name) {
        reset.write(rst ? SC_LOGIC_1 : SC_LOGIC_0); x.write(xv ? SC_LOGIC_1 : SC_LOGIC_0);
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        reference.tick(x.read(), rst);
        const auto expected = reference.output(x.read()), actual = y.read();
        const bool pass = expected == actual;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\"," << x.read() << ',' << rst << ','
            << expected << ',' << actual << ',' << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " x=" << x.read() << " reset=" << rst
                  << " expected_y=" << expected << " actual_y=" << actual
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " x=" << x.read() << " reset=" << rst
                             << " expected_y=" << expected << " actual_y=" << actual << '\n';
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    reset.write(SC_LOGIC_1); x.write(SC_LOGIC_0); clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    run_custdesign_stimulus(cycle);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures
              << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
