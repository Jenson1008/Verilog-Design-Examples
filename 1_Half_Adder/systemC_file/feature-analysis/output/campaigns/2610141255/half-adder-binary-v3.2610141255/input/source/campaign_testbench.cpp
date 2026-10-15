#include <systemc>

#include <fstream>
#include <iostream>
#include <string>

#include "DUT/half_adder_sc.h"

namespace {

bool read_argument(int argc, char** argv, const std::string& name, std::string& value) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == name) {
            value = argv[i + 1];
            return true;
        }
    }
    return false;
}

}  // namespace

SC_MODULE(half_adder_campaign_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a{"i_a"}, i_b{"i_b"};
    sc_core::sc_in<sc_dt::sc_logic> o_sum{"o_sum"}, o_carry{"o_carry"};
    std::string test_id;
    std::string run_id;
    std::string cycle_semantics_version;
    unsigned failures = 0;

    void run() {
        std::ofstream csv("half_adder.csv", std::ios::out | std::ios::trunc);
        if (!csv) {
            std::cerr << "ERROR: cannot create half_adder.csv\n";
            failures = 1;
            sc_core::sc_stop();
            return;
        }

        csv << "test_id,run_id,cycle_semantics_version,i_a,i_b,expected_o_sum,actual_o_sum,expected_o_carry,actual_o_carry,status\n";
        const int vectors[4][4] = {
            {0, 0, 0, 0},
            {0, 1, 1, 0},
            {1, 0, 1, 0},
            {1, 1, 0, 1},
        };
        for (const auto& vector : vectors) {
            i_a.write(vector[0] ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
            i_b.write(vector[1] ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
            wait(1, sc_core::SC_NS);

            const char expected_sum = static_cast<char>('0' + vector[2]);
            const char expected_carry = static_cast<char>('0' + vector[3]);
            const char actual_sum = o_sum.read().to_char();
            const char actual_carry = o_carry.read().to_char();
            const bool pass = expected_sum == actual_sum && expected_carry == actual_carry;
            if (!pass) {
                ++failures;
            }
            const char* status = pass ? "PASS" : "FAIL";
            csv << test_id << ',' << run_id << ',' << cycle_semantics_version << ','
                << static_cast<char>('0' + vector[0]) << ','
                << static_cast<char>('0' + vector[1]) << ','
                << expected_sum << ',' << actual_sum << ','
                << expected_carry << ',' << actual_carry << ',' << status << '\n';
        }

        csv.flush();
        if (!csv) {
            std::cerr << "ERROR: failed to write half_adder.csv\n";
            ++failures;
        }
        std::cout << "SUMMARY checks=4 failures=" << failures << '\n';
        sc_core::sc_stop();
    }

    SC_CTOR(half_adder_campaign_stimulus) {
        SC_THREAD(run);
    }
};

int sc_main(int argc, char** argv) {
    std::string test_id;
    std::string run_id;
    std::string cycle_semantics_version;
    if (!read_argument(argc, argv, "--c2s-test-id", test_id)
        || !read_argument(argc, argv, "--c2s-run-id", run_id)
        || !read_argument(argc, argv, "--c2s-cycle-semantics-version", cycle_semantics_version)) {
        std::cerr << "ERROR: missing C2S trace identity argument\n";
        return 2;
    }

    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    sc_core::sc_signal<sc_dt::sc_logic> a("i_a"), b("i_b"), sum("o_sum"), carry("o_carry");
    half_adder dut("dut");
    half_adder_campaign_stimulus stimulus("stimulus");
    dut.i_a(a);
    dut.i_b(b);
    dut.o_sum(sum);
    dut.o_carry(carry);
    stimulus.i_a(a);
    stimulus.i_b(b);
    stimulus.o_sum(sum);
    stimulus.o_carry(carry);
    stimulus.test_id = test_id;
    stimulus.run_id = run_id;
    stimulus.cycle_semantics_version = cycle_semantics_version;

    auto* vcd = sc_core::sc_create_vcd_trace_file("half_adder_wave");
    if (!vcd) {
        std::cerr << "ERROR: cannot create half_adder_wave.vcd\n";
        return 1;
    }
    vcd->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(vcd, a, "i_a");
    sc_core::sc_trace(vcd, b, "i_b");
    sc_core::sc_trace(vcd, sum, "o_sum");
    sc_core::sc_trace(vcd, carry, "o_carry");
    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(vcd);
    return stimulus.failures == 0 ? 0 : 1;
}
