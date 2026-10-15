#include "DUT/singleport_ram_sc.h"
#include "stimulus/singleport_ram_sc_stimulus.h"
#include <array>
#include <fstream>
#include <iostream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_lv<128>> data_in("data_in"), data_out("data_out");
    sc_signal<sc_dt::sc_logic> wr_en("wr_en"), clk("clk");
    sc_signal<sc_dt::sc_lv<3>> addr("addr");
    singleport_ram dut("dut");
    dut.data_in(data_in); dut.wr_en(wr_en); dut.addr(addr); dut.clk(clk); dut.data_out(data_out);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,addr,wr_en,data_in,expected_data_out,actual_data_out,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, data_in, "data_in"); sc_trace(vcd, data_out, "data_out");
    sc_trace(vcd, wr_en, "wr_en"); sc_trace(vcd, addr, "addr"); sc_trace(vcd, clk, "clk");
    std::array<sc_dt::sc_lv<128>, 8> oracle;
    unsigned checks = 0, failures = 0;
    auto transact = [&](unsigned a, bool write, const sc_dt::sc_lv<128>& d, const std::string& name) {
        clk.write(SC_LOGIC_0); addr.write(a); wr_en.write(write ? SC_LOGIC_1 : SC_LOGIC_0); data_in.write(d);
        sc_start(1, SC_NS);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        if (write) oracle[a] = d;
        const auto expected = oracle[a], actual = data_out.read();
        const bool pass = actual == expected;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\"," << a << ',' << write << ",\"" << d << "\",\""
            << expected << "\",\"" << actual << "\"," << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " addr=" << a << " wr_en=" << write
                  << " data_in=" << d
                  << " expected_data_out=" << expected << " actual_data_out=" << actual
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " addr=" << a << " wr_en=" << write << " data_in=" << d
                             << " expected_data_out=" << expected << " actual_data_out=" << actual << '\n';
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    run_singleport_ram_stimulus(transact);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures
              << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
