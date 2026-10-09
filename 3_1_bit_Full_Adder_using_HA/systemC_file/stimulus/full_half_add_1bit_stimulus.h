#ifndef FULL_HALF_ADD_1BIT_STIMULUS_H
#define FULL_HALF_ADD_1BIT_STIMULUS_H

#include <systemc>

#include <array>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(full_half_add_1bit_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a;
    sc_core::sc_out<sc_dt::sc_logic> i_b;
    sc_core::sc_out<sc_dt::sc_logic> i_cin;
    sc_core::sc_in<sc_dt::sc_logic> o_sum;
    sc_core::sc_in<sc_dt::sc_logic> o_carry;

    SC_HAS_PROCESS(full_half_add_1bit_stimulus);

    full_half_add_1bit_stimulus(
        sc_core::sc_module_name name,
        const std::string& log_path,
        const std::string& vcd_path,
        bool include_four_state_cases)
        : sc_core::sc_module(name),
          log_(log_path),
          log_path_(log_path),
          vcd_path_(vcd_path),
          include_four_state_cases_(include_four_state_cases) {
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

    unsigned int checks() const {
        return cycle_;
    }

    unsigned int failed_checks() const {
        return failed_checks_;
    }

private:
    struct FourStateCase {
        const char* name;
        sc_dt::sc_logic a;
        sc_dt::sc_logic b;
        sc_dt::sc_logic cin;
        sc_dt::sc_logic sum;
        sc_dt::sc_logic carry;
    };

    std::ofstream log_;
    std::string log_path_;
    std::string vcd_path_;
    bool include_four_state_cases_;
    bool log_ready_ = false;
    bool failed_ = false;
    unsigned int cycle_ = 0;
    unsigned int failed_checks_ = 0;

    void run() {
        if (!log_ready_) {
            failed_ = true;
            std::cerr << "ERROR: cannot open or initialize simulation CSV log\n";
            return;
        }

        for (unsigned int value = 0; value < 8; ++value) {
            const unsigned int a = (value >> 2U) & 1U;
            const unsigned int b = (value >> 1U) & 1U;
            const unsigned int cin = value & 1U;
            const unsigned int total = a + b + cin;

            i_a.write(a == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            i_b.write(b == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            i_cin.write(cin == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1);
            wait(1, sc_core::SC_NS);

            record(
                "binary_truth_table",
                a == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1,
                b == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1,
                cin == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1,
                (total & 1U) == 0U ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_1,
                total >= 2U ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        }

        if (include_four_state_cases_) {
            const std::array<FourStateCase, 4> cases{{
                {"x_input_with_zero", sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X,
                 sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_0},
                {"z_input_with_carry", sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_Z,
                 sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X},
                {"x_carry_in_after_two_ones", sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_1,
                 sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_1},
                {"x_input_with_one_zero", sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_0,
                 sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_0},
            }};

            for (const auto& test : cases) {
                i_a.write(test.a);
                i_b.write(test.b);
                i_cin.write(test.cin);
                wait(1, sc_core::SC_NS);
                record(test.name, test.a, test.b, test.cin, test.sum, test.carry);
            }
        }

        log_ << "# SUMMARY: " << (failed_ ? "FAIL" : "PASS")
             << " checks=" << cycle_ << " failed=" << failed_checks_
             << " csv=" << log_path_
             << " vcd=" << vcd_path_ << '\n';
        log_.close();
        if (!log_.good()) {
            failed_ = true;
            std::cerr << "ERROR: failed while finalizing simulation CSV log\n";
        }

        const bool passed = !failed_;
        std::cout << "# SUMMARY: " << (passed ? "PASS" : "FAIL")
                  << " checks=" << cycle_
                  << " failed=" << failed_checks_ << '\n';
        std::cout << "CSV log: " << log_path_ << '\n';
        std::cout << "VCD waveform: " << vcd_path_ << '\n';
    }

    void record(
        const char* test_name,
        const sc_dt::sc_logic& a,
        const sc_dt::sc_logic& b,
        const sc_dt::sc_logic& cin,
        const sc_dt::sc_logic& expected_sum,
        const sc_dt::sc_logic& expected_carry) {
        const sc_dt::sc_logic actual_sum = o_sum.read();
        const sc_dt::sc_logic actual_carry = o_carry.read();
        const bool pass = actual_sum == expected_sum && actual_carry == expected_carry;
        const char* result = pass ? "PASS" : "FAIL";
        const double time_ns = sc_core::sc_time_stamp().to_seconds() * 1.0e9;

        log_ << time_ns << ',' << test_name << ',' << cycle_ << ','
             << a.to_char() << ',' << b.to_char() << ',' << cin.to_char() << ','
             << expected_sum.to_char() << ',' << expected_carry.to_char() << ','
             << actual_sum.to_char() << ',' << actual_carry.to_char() << ','
             << result << '\n';
        std::cout << "time=" << time_ns << " ns"
                  << " test=" << test_name
                  << " cycle=" << cycle_
                  << " inputs=" << a.to_char() << b.to_char() << cin.to_char()
                  << " expected_sum=" << expected_sum.to_char()
                  << " expected_carry=" << expected_carry.to_char()
                  << " actual_sum=" << actual_sum.to_char()
                  << " actual_carry=" << actual_carry.to_char()
                  << " result=" << result << '\n';

        if (!log_.good()) {
            failed_ = true;
            std::cerr << "ERROR: failed writing simulation CSV log at cycle "
                      << cycle_ << '\n';
        }

        if (!pass) {
            failed_ = true;
            ++failed_checks_;
            std::cerr << "FAIL time=" << time_ns << " ns"
                      << " test=" << test_name
                      << " inputs=" << a.to_char() << b.to_char() << cin.to_char()
                      << " expected=" << expected_sum.to_char()
                      << expected_carry.to_char()
                      << " actual=" << actual_sum.to_char()
                      << actual_carry.to_char() << '\n';
        }
        ++cycle_;
    }
};

#endif
