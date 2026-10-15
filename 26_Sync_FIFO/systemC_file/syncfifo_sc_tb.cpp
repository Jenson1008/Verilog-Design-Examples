#include "DUT/syncfifo_sc.h"
#include "stimulus/syncfifo_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_logic> clk("clk"), reset("reset"), write_en("write_en"), read_en("read_en"), full("full"), empty("empty");
    sc_signal<sc_dt::sc_lv<8>> data_in("data_in"), out("out");
    syncfifo dut("dut"); dut.clk(clk); dut.reset(reset); dut.write_en(write_en); dut.read_en(read_en);
    dut.data_in(data_in); dut.full(full); dut.empty(empty); dut.out(out);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,write_en,read_en,data_in,expected_full,actual_full,expected_empty,actual_empty,expected_out,actual_out,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, clk, "clk"); sc_trace(vcd, reset, "reset"); sc_trace(vcd, write_en, "write_en");
    sc_trace(vcd, read_en, "read_en"); sc_trace(vcd, data_in, "data_in"); sc_trace(vcd, full, "full");
    sc_trace(vcd, empty, "empty"); sc_trace(vcd, out, "out");
    sc_trace(vcd, dut.write_pointer, "dut.write_pointer"); sc_trace(vcd, dut.read_pointer, "dut.read_pointer");
    sc_trace(vcd, dut.count, "dut.count");
    for (unsigned i = 0; i < 8; ++i)
        sc_trace(vcd, dut.memory[i], ("dut.memory_" + std::to_string(i)).c_str());
    SyncFifoReference ref;
    unsigned checks = 0, failures = 0;
    auto cycle = [&](bool wr, bool rd, unsigned value, const std::string& name) {
        clk.write(SC_LOGIC_0); write_en.write(wr ? SC_LOGIC_1 : SC_LOGIC_0); read_en.write(rd ? SC_LOGIC_1 : SC_LOGIC_0);
        data_in.write(value); sc_start(1, SC_NS);
        ref.step(wr, rd, value);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        const auto ef = ref.queue.size() == 8 ? SC_LOGIC_1 : SC_LOGIC_0;
        const auto ee = ref.queue.empty() ? SC_LOGIC_1 : SC_LOGIC_0;
        const sc_dt::sc_lv<8> eo(ref.last_out);
        const bool pass = full.read() == ef && empty.read() == ee && (!ref.out_known || out.read() == eo);
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        std::ostringstream expected_text;
        if (ref.out_known) expected_text << eo;
        else expected_text << "unknown (uninitialized/held)";
        const std::string output_expected = expected_text.str();
        csv << t << ",\"" << name << "\"," << wr << ',' << rd << ',' << value << ',' << ef << ',' << full.read()
            << ',' << ee << ',' << empty.read() << ",\"" << output_expected << "\",\"" << out.read() << "\","
            << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " write_en=" << wr << " read_en=" << rd
                  << " data_in=" << value << " expected_full=" << ef << " actual_full=" << full.read()
                  << " expected_empty=" << ee << " actual_empty=" << empty.read()
                  << " expected_out=" << output_expected << " actual_out=" << out.read()
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " write_en=" << wr << " read_en=" << rd << " data_in=" << value
                             << " expected_full=" << ef << " actual_full=" << full.read()
                             << " expected_empty=" << ee << " actual_empty=" << empty.read()
                             << " expected_out=" << output_expected << " actual_out=" << out.read() << '\n';
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    reset.write(SC_LOGIC_1); clk.write(SC_LOGIC_0); write_en.write(SC_LOGIC_0); read_en.write(SC_LOGIC_0); sc_start(1, SC_NS);
    reset.write(SC_LOGIC_0); sc_start(1, SC_NS); // asynchronous active-low clear
    reset.write(SC_LOGIC_1); sc_start(1, SC_NS);
    run_syncfifo_stimulus(cycle);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
