#include "DUT/seqmult_sc.h"
#include "stimulus/seqmult_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_lv<8>> in_a("in_a"), in_b("in_b");
    sc_signal<sc_dt::sc_logic> clk("clk"), load("load"), reset("reset"), out_valid("out_valid");
    sc_signal<sc_dt::sc_lv<16>> out_prod("out_prod");
    seqmult dut("dut"); dut.in_a(in_a); dut.in_b(in_b); dut.clk(clk); dut.load(load); dut.reset(reset);
    dut.out_valid(out_valid); dut.out_prod(out_prod);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,in_a,in_b,load,expected_valid,actual_valid,expected_out_prod,actual_out_prod,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, in_a, "in_a"); sc_trace(vcd, in_b, "in_b"); sc_trace(vcd, clk, "clk");
    sc_trace(vcd, load, "load"); sc_trace(vcd, reset, "reset"); sc_trace(vcd, out_valid, "out_valid"); sc_trace(vcd, out_prod, "out_prod");
    unsigned checks = 0, failures = 0;
    bool output_known = false, bank_known = false;
    unsigned bank_product = 0, output_product = 0;
    auto check = [&](const std::string& name, unsigned a, unsigned b, bool ld, bool valid_expected,
                     const std::string& expected_string) {
        bool pass = out_valid.read() == (valid_expected ? SC_LOGIC_1 : SC_LOGIC_0);
        if (expected_string == "Z") pass = pass && out_prod.read() == sc_dt::sc_lv<16>(sc_dt::SC_LOGIC_Z);
        else if (expected_string != "X") {
            const unsigned expected = expected_string == "bank" ? bank_product : output_product;
            pass = pass && out_prod.read() == sc_dt::sc_lv<16>(expected);
        }
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\"," << a << ',' << b << ',' << ld << ',' << valid_expected << ','
            << out_valid.read() << ",\"" << expected_string << "\",\"" << out_prod.read() << "\","
            << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " in_a=" << a << " in_b=" << b << " load=" << ld
                  << " expected_out_valid=" << valid_expected << " actual_out_valid=" << out_valid.read()
                  << " expected_out_prod=" << expected_string << " actual_out_prod=" << out_prod.read()
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " in_a=" << a << " in_b=" << b << " load=" << ld
                             << " expected_valid=" << valid_expected << " actual_valid=" << out_valid.read()
                             << " expected_out_prod=" << expected_string << " actual_out_prod=" << out_prod.read() << '\n';
    };
    auto cycle = [&](unsigned a, unsigned b, bool ld, const std::string& name) {
        clk.write(SC_LOGIC_0); in_a.write(a); in_b.write(b); load.write(ld ? SC_LOGIC_1 : SC_LOGIC_0); sc_start(1, SC_NS);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        if (ld) {
            if (bank_known) { output_product = bank_product; output_known = true; }
            if (output_known) check(name, a, b, ld, true, "bank");
            else check(name, a, b, ld, true, "X");
            bank_product = seqmult_reference(a, b); bank_known = true;
        } else {
            check(name, a, b, ld, output_known, output_known ? "known" : "Z");
        }
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    clk.write(SC_LOGIC_0); reset.write(SC_LOGIC_0); load.write(SC_LOGIC_0); sc_start(1, SC_NS);
    reset.write(SC_LOGIC_1); sc_start(1, SC_NS);
    check("async-reset-z", 0, 0, false, false, "Z");
    reset.write(SC_LOGIC_0); sc_start(1, SC_NS);
    run_seqmult_stimulus(cycle);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
