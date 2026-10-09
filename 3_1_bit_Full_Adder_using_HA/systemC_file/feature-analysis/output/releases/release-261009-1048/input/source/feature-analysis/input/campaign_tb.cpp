#include "../../DUT/full_half_add_1bit.h"

#include <array>
#include <fstream>
#include <iostream>

SC_MODULE(campaign_driver) {
    sc_core::sc_out<sc_dt::sc_logic> i_a;
    sc_core::sc_out<sc_dt::sc_logic> i_b;
    sc_core::sc_out<sc_dt::sc_logic> i_cin;
    sc_core::sc_in<sc_dt::sc_logic> o_sum;
    sc_core::sc_in<sc_dt::sc_logic> o_carry;

    SC_HAS_PROCESS(campaign_driver);

    explicit campaign_driver(sc_core::sc_module_name name)
        : sc_core::sc_module(name), log_("full_adder_campaign.csv") {
        if (log_) {
            log_ << "time_ns,test,cycle,i_a,i_b,i_cin,expected_o_sum,"
                    "expected_o_carry,o_sum,o_carry,result\n";
        }
        log_ready_ = log_.good();
        SC_THREAD(run);
    }

    bool log_ready() const {
        return log_ready_;
    }

    bool failed() const {
        return failed_;
    }

private:
    std::ofstream log_;
    bool log_ready_ = false;
    bool failed_ = false;
    unsigned int failed_checks_ = 0;

    void run() {
        if (!log_ready_) {
            failed_ = true;
            std::cerr << "ERROR: cannot open full_adder_campaign.csv\n";
            return;
        }

        for (unsigned int cycle = 0; cycle < 8; ++cycle) {
            const unsigned int a = (cycle >> 2U) & 1U;
            const unsigned int b = (cycle >> 1U) & 1U;
            const unsigned int cin = cycle & 1U;
            const unsigned int total = a + b + cin;
            const unsigned int expected_sum = total & 1U;
            const unsigned int expected_carry = total >> 1U;

            i_a.write(a == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            i_b.write(b == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            i_cin.write(cin == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            wait(1, sc_core::SC_NS);

            const sc_dt::sc_logic actual_sum = o_sum.read();
            const sc_dt::sc_logic actual_carry = o_carry.read();
            const bool pass =
                actual_sum == (expected_sum == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1) &&
                actual_carry == (expected_carry == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            const char* result = pass ? "PASS" : "FAIL";
            const double time_ns = sc_core::sc_time_stamp().to_seconds() * 1.0e9;

            log_ << time_ns << ",binary_truth_table," << cycle << ','
                 << a << ',' << b << ',' << cin << ','
                 << expected_sum << ',' << expected_carry << ','
                 << actual_sum.to_char() << ',' << actual_carry.to_char() << ','
                 << result << '\n';
            std::cout << "time=" << time_ns << " ns"
                      << " test=binary_truth_table"
                      << " cycle=" << cycle
                      << " inputs=" << a << b << cin
                      << " expected_sum=" << expected_sum
                      << " expected_carry=" << expected_carry
                      << " actual_sum=" << actual_sum.to_char()
                      << " actual_carry=" << actual_carry.to_char()
                      << " result=" << result << '\n';

            if (!log_.good()) {
                failed_ = true;
                std::cerr << "ERROR: failed writing CSV row at cycle " << cycle << '\n';
            }
            if (!pass) {
                failed_ = true;
                ++failed_checks_;
                std::cerr << "FAIL time=" << time_ns << " ns cycle=" << cycle
                          << " inputs=" << a << b << cin
                          << " expected=" << expected_sum << expected_carry
                          << " actual=" << actual_sum.to_char() << actual_carry.to_char()
                          << '\n';
            }
        }

        log_ << "# SUMMARY: " << (failed_ ? "FAIL" : "PASS")
             << " checks=8 failed=" << failed_checks_
             << " csv=full_adder_campaign.csv"
             << " vcd=full_adder_campaign_wave.vcd\n";
        log_.close();
        if (!log_.good()) {
            failed_ = true;
            std::cerr << "ERROR: failed finalizing CSV log\n";
        }
        std::cout << "# SUMMARY: " << (failed_ ? "FAIL" : "PASS")
                  << " checks=8 failed=" << failed_checks_ << '\n';
        std::cout << "CSV log: full_adder_campaign.csv\n"
                     "VCD waveform: full_adder_campaign_wave.vcd\n";
    }
};

int sc_main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);

    sc_core::sc_signal<sc_dt::sc_logic> sig_i_a{"sig_i_a"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_i_b{"sig_i_b"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_i_cin{"sig_i_cin"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_sum{"sig_o_sum"};
    sc_core::sc_signal<sc_dt::sc_logic> sig_o_carry{"sig_o_carry"};

    full_half_add_1bit dut{"full_half_add_1bit"};
    campaign_driver driver{"campaign_driver"};

    dut.i_a(sig_i_a);
    dut.i_b(sig_i_b);
    dut.i_cin(sig_i_cin);
    dut.o_sum(sig_o_sum);
    dut.o_carry(sig_o_carry);
    driver.i_a(sig_i_a);
    driver.i_b(sig_i_b);
    driver.i_cin(sig_i_cin);
    driver.o_sum(sig_o_sum);
    driver.o_carry(sig_o_carry);

    if (!driver.log_ready()) {
        std::cerr << "ERROR: cannot initialize campaign CSV log\n";
        return 2;
    }

    sc_core::sc_trace_file* trace =
        sc_core::sc_create_vcd_trace_file("full_adder_campaign_wave");
    if (trace == nullptr) {
        std::cerr << "ERROR: cannot create campaign VCD\n";
        return 2;
    }
    trace->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace, sig_i_a, "i_a");
    sc_core::sc_trace(trace, sig_i_b, "i_b");
    sc_core::sc_trace(trace, sig_i_cin, "i_cin");
    sc_core::sc_trace(trace, sig_o_sum, "o_sum");
    sc_core::sc_trace(trace, sig_o_carry, "o_carry");
    sc_core::sc_trace(trace, dut.w_sum1, "w_sum1");
    sc_core::sc_trace(trace, dut.w_carry1, "w_carry1");
    sc_core::sc_trace(trace, dut.w_carry2, "w_carry2");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace);
    return driver.failed() ? 1 : 0;
}
