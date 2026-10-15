#include "DUT/asyncfifo_sc.h"
#include "stimulus/asyncfifo_sc_stimulus.h"
#include <fstream>
#include <iostream>
#include <string>

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);
    sc_signal<sc_dt::sc_logic> write_clk("write_clk"), read_clk("read_clk"), reset("reset");
    sc_signal<sc_dt::sc_logic> write_en("write_en"), read_en("read_en"), mem_full("mem_full"), mem_empty("mem_empty");
    sc_signal<sc_dt::sc_lv<8>> data_in("data_in"), out("out");
    asyncfifo dut("dut"); dut.write_clk(write_clk); dut.read_clk(read_clk); dut.reset(reset);
    dut.write_en(write_en); dut.read_en(read_en); dut.data_in(data_in); dut.mem_full(mem_full); dut.mem_empty(mem_empty); dut.out(out);
    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,write_en,read_en,data_in,expected_full,actual_full,expected_empty,actual_empty,expected_out,actual_out,status\n";
    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, write_clk, "write_clk"); sc_trace(vcd, read_clk, "read_clk"); sc_trace(vcd, reset, "reset");
    sc_trace(vcd, write_en, "write_en"); sc_trace(vcd, read_en, "read_en"); sc_trace(vcd, data_in, "data_in");
    sc_trace(vcd, mem_full, "mem_full"); sc_trace(vcd, mem_empty, "mem_empty"); sc_trace(vcd, out, "out");
    sc_trace(vcd, dut.write_ptr, "dut.write_ptr"); sc_trace(vcd, dut.read_ptr, "dut.read_ptr");
    sc_trace(vcd, dut.count, "dut.count");
    for (unsigned i = 0; i < 8; ++i)
        sc_trace(vcd, dut.mem[i], ("dut.mem_" + std::to_string(i)).c_str());
    AsyncFifoReference ref;
    unsigned checks = 0, failures = 0;
    auto check = [&](const std::string& name, const std::string& expected_out) {
        const auto ef = ref.queue.size() == 8 ? SC_LOGIC_1 : SC_LOGIC_0;
        const auto ee = ref.queue.empty() ? SC_LOGIC_1 : SC_LOGIC_0;
        bool output_ok = true;
        if (expected_out == "Z") output_ok = out.read() == sc_dt::sc_lv<8>(sc_dt::SC_LOGIC_Z);
        else if (expected_out != "hold") output_ok = out.read() == sc_dt::sc_lv<8>(ref.last_out);
        const bool pass = mem_full.read() == ef && mem_empty.read() == ee && output_ok;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"" << name << "\"," << write_en.read() << ',' << read_en.read() << ",\"" << data_in.read()
            << "\"," << ef << ',' << mem_full.read() << ',' << ee << ',' << mem_empty.read()
            << ",\"" << expected_out << "\",\"" << out.read() << "\"," << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns " << name << " write_en=" << write_en.read() << " read_en=" << read_en.read()
                  << " data_in=" << data_in.read()
                  << " expected_full=" << ef << " actual_full=" << mem_full.read()
                  << " expected_empty=" << ee << " actual_empty=" << mem_empty.read()
                  << " expected_out=" << expected_out << " actual_out=" << out.read()
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL " << name << " at " << sc_time_stamp()
                             << " write_en=" << write_en.read() << " read_en=" << read_en.read()
                             << " data_in=" << data_in.read() << " expected_full=" << ef
                             << " actual_full=" << mem_full.read() << " expected_empty=" << ee
                             << " actual_empty=" << mem_empty.read() << " expected_out=" << expected_out
                             << " actual_out=" << out.read() << '\n';
    };
    write_clk.write(SC_LOGIC_0); read_clk.write(SC_LOGIC_0); reset.write(SC_LOGIC_1);
    write_en.write(SC_LOGIC_0); read_en.write(SC_LOGIC_0); data_in.write(0); sc_start(1, SC_NS);
    reset.write(SC_LOGIC_0); sc_start(1, SC_NS);
    reset.write(SC_LOGIC_1); sc_start(1, SC_NS); check("async-reset", "hold");

    auto write_one = [&](unsigned i, unsigned value) {
        write_en.write(SC_LOGIC_1); read_en.write(SC_LOGIC_0); data_in.write(value);
        write_clk.write(SC_LOGIC_1); sc_start(1, SC_NS); ref.write(value); check("write-" + std::to_string(i), "Z");
        write_clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    auto check_full = [&]() { check("full-after-eight-writes", "Z"); };
    auto read_one = [&](unsigned i) {
        write_en.write(SC_LOGIC_0); read_en.write(SC_LOGIC_1);
        read_clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        const unsigned expected = ref.queue.front(); ref.read();
        const bool pass_data = out.read() == sc_dt::sc_lv<8>(expected);
        mem_full.read(); mem_empty.read();
        const auto ef = ref.queue.size() == 8 ? SC_LOGIC_1 : SC_LOGIC_0;
        const auto ee = ref.queue.empty() ? SC_LOGIC_1 : SC_LOGIC_0;
        const bool pass = pass_data && mem_full.read() == ef && mem_empty.read() == ee;
        ++checks; failures += !pass;
        const auto t = sc_time_stamp().to_seconds() * 1.0e9;
        csv << t << ",\"read-" << i << "\",0,1,\"" << data_in.read() << "\"," << ef << ',' << mem_full.read()
            << ',' << ee << ',' << mem_empty.read() << ",\"" << expected << "\",\"" << out.read() << "\","
            << (pass ? "PASS" : "FAIL") << '\n';
        std::cout << "t=" << t << " ns read-" << i << " write_en=0 read_en=1 data_in=" << data_in.read()
                  << " expected_out=" << expected
                  << " actual_out=" << out.read() << " expected_empty=" << ee << " actual_empty=" << mem_empty.read()
                  << " " << (pass ? "PASS" : "FAIL") << '\n';
        if (!pass) std::cerr << "FAIL read-" << i << " at " << sc_time_stamp()
                             << " write_en=0 read_en=1 data_in=" << data_in.read()
                             << " expected_out=" << expected << " actual_out=" << out.read()
                             << " expected_full=" << ef << " actual_full=" << mem_full.read()
                             << " expected_empty=" << ee << " actual_empty=" << mem_empty.read() << '\n';
        read_clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };
    auto check_empty = [&]() {
        write_en.write(SC_LOGIC_0); read_en.write(SC_LOGIC_0); check("empty-after-drain", "hold");
    };
    run_asyncfifo_stimulus(write_one, check_full, read_one, check_empty);
    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close(); sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " csv=checks.csv vcd=wave.vcd\n";
    return failures ? 1 : 0;
}
