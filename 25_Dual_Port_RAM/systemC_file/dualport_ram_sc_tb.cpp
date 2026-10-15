#include "DUT/dualport_ram_sc.h"
#include "stimulus/dualport_ram_sc_stimulus.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

struct DualPortRamOracle {
    std::array<sc_dt::sc_lv<128>, 8> memory;
    sc_dt::sc_lv<128> output_a;
    sc_dt::sc_lv<128> output_b;

    DualPortRamOracle() {
        for (auto& word : memory)
            for (unsigned bit = 0; bit < 128; ++bit) word[bit] = sc_dt::SC_LOGIC_X;
        for (unsigned bit = 0; bit < 128; ++bit) {
            output_a[bit] = sc_dt::SC_LOGIC_X;
            output_b[bit] = sc_dt::SC_LOGIC_X;
        }
    }

    void advance(const DualPortRamTransaction& tx) {
        const auto old_memory = memory;
        if (!tx.write_a) output_a = old_memory[tx.address_a];
        if (!tx.write_b) output_b = old_memory[tx.address_b];

        if (tx.write_a && tx.write_b) {
            if (tx.address_a == tx.address_b) {
                for (unsigned bit = 0; bit < 128; ++bit)
                    memory[tx.address_a][bit] = sc_dt::SC_LOGIC_X;
            } else {
                memory[tx.address_a] = tx.input_a;
                memory[tx.address_b] = tx.input_b;
            }
        } else if (tx.write_a) {
            memory[tx.address_a] = tx.input_a;
        } else if (tx.write_b) {
            memory[tx.address_b] = tx.input_b;
        }
    }
};

static std::string bits(const sc_dt::sc_lv<128>& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

int sc_main(int, char**) {
    using namespace sc_core;
    using namespace sc_dt;
    sc_set_time_resolution(1, SC_PS);

    sc_signal<sc_lv<128>> data_in_a("data_in_a"), data_in_b("data_in_b");
    sc_signal<sc_lv<128>> data_out_a("data_out_a"), data_out_b("data_out_b");
    sc_signal<sc_lv<3>> addr_a("addr_a"), addr_b("addr_b");
    sc_signal<sc_logic> wr_en_a("wr_en_a"), wr_en_b("wr_en_b"), clk("clk");
    dualport_ram dut("dut");
    dut.data_in_a(data_in_a); dut.data_in_b(data_in_b);
    dut.wr_en_a(wr_en_a); dut.wr_en_b(wr_en_b); dut.addr_a(addr_a); dut.addr_b(addr_b);
    dut.clk(clk); dut.data_out_a(data_out_a); dut.data_out_b(data_out_b);

    std::ofstream csv("checks.csv", std::ios::trunc);
    if (!csv) { std::cerr << "cannot open checks.csv\n"; return 2; }
    csv << "time_ns,test,wr_en_a,addr_a,data_in_a,wr_en_b,addr_b,data_in_b,expected_data_out_a,actual_data_out_a,expected_data_out_b,actual_data_out_b,status\n";

    auto* vcd = sc_create_vcd_trace_file("wave");
    if (!vcd) { std::cerr << "cannot create wave.vcd\n"; return 2; }
    vcd->set_time_unit(1, SC_NS);
    sc_trace(vcd, clk, "clk"); sc_trace(vcd, wr_en_a, "wr_en_a"); sc_trace(vcd, addr_a, "addr_a");
    sc_trace(vcd, data_in_a, "data_in_a"); sc_trace(vcd, data_out_a, "data_out_a");
    sc_trace(vcd, wr_en_b, "wr_en_b"); sc_trace(vcd, addr_b, "addr_b");
    sc_trace(vcd, data_in_b, "data_in_b"); sc_trace(vcd, data_out_b, "data_out_b");
    for (unsigned i = 0; i < 8; ++i)
        sc_trace(vcd, dut.ram_vector[i], ("dut.ram_vector_" + std::to_string(i)).c_str());

    DualPortRamOracle oracle;
    unsigned checks = 0, failures = 0;
    auto transaction = [&](const DualPortRamTransaction& tx) {
        clk.write(SC_LOGIC_0);
        wr_en_a.write(tx.write_a ? SC_LOGIC_1 : SC_LOGIC_0);
        wr_en_b.write(tx.write_b ? SC_LOGIC_1 : SC_LOGIC_0);
        addr_a.write(tx.address_a); addr_b.write(tx.address_b);
        data_in_a.write(tx.input_a); data_in_b.write(tx.input_b);
        sc_start(1, SC_NS);

        // The oracle snapshots old contents, then applies both ports without
        // assigning a priority to a same-address dual write.
        oracle.advance(tx);
        clk.write(SC_LOGIC_1); sc_start(1, SC_NS);
        const auto actual_a = data_out_a.read(), actual_b = data_out_b.read();
        const bool pass = actual_a == oracle.output_a && actual_b == oracle.output_b;
        ++checks; failures += !pass;
        const double time_ns = sc_time_stamp().to_seconds() * 1.0e9;
        const auto status = pass ? "PASS" : "FAIL";
        csv << time_ns << ",\"" << tx.name << "\"," << tx.write_a << ',' << tx.address_a << ",\""
            << tx.input_a << "\"," << tx.write_b << ',' << tx.address_b << ",\"" << tx.input_b
            << "\",\"" << oracle.output_a << "\",\"" << actual_a << "\",\""
            << oracle.output_b << "\",\"" << actual_b << "\"," << status << '\n';
        std::cout << "t=" << time_ns << " ns " << tx.name
                  << " wr_en_a=" << tx.write_a << " addr_a=" << tx.address_a << " data_in_a=" << tx.input_a
                  << " wr_en_b=" << tx.write_b << " addr_b=" << tx.address_b << " data_in_b=" << tx.input_b
                  << " expected_data_out_a=" << oracle.output_a << " actual_data_out_a=" << actual_a
                  << " expected_data_out_b=" << oracle.output_b << " actual_data_out_b=" << actual_b
                  << ' ' << status << '\n';
        if (!pass)
            std::cerr << "FAIL " << tx.name << " at " << sc_time_stamp()
                      << " wr_en_a=" << tx.write_a << " addr_a=" << tx.address_a << " data_in_a=" << bits(tx.input_a)
                      << " wr_en_b=" << tx.write_b << " addr_b=" << tx.address_b << " data_in_b=" << bits(tx.input_b)
                      << " expected_data_out_a=" << bits(oracle.output_a) << " actual_data_out_a=" << bits(actual_a)
                      << " expected_data_out_b=" << bits(oracle.output_b) << " actual_data_out_b=" << bits(actual_b) << '\n';
        clk.write(SC_LOGIC_0); sc_start(1, SC_NS);
    };

    clk.write(SC_LOGIC_0); wr_en_a.write(SC_LOGIC_0); wr_en_b.write(SC_LOGIC_0);
    addr_a.write(0); addr_b.write(0); data_in_a.write(0); data_in_b.write(0); sc_start(1, SC_NS);
    run_dualport_ram_stimulus(transaction);

    csv << "#summary,checks=" << checks << ",failures=" << failures << '\n';
    csv.close();
    const bool log_ok = static_cast<bool>(csv);
    sc_close_vcd_trace_file(vcd);
    std::cout << "SUMMARY checks=" << checks << " failures=" << failures
              << " csv=checks.csv vcd=wave.vcd\n";
    if (!log_ok) { std::cerr << "failed writing checks.csv\n"; return 2; }
    return failures ? 1 : 0;
}
