#ifndef HALF_ADDER_SC_STIMULUS_H
#define HALF_ADDER_SC_STIMULUS_H

#include <systemc>

#include <array>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(half_adder_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a;
    sc_core::sc_out<sc_dt::sc_logic> i_b;
    sc_core::sc_in<sc_dt::sc_logic> o_sum;
    sc_core::sc_in<sc_dt::sc_logic> o_carry;

    SC_HAS_PROCESS(half_adder_stimulus);

    half_adder_stimulus(sc_core::sc_module_name name, const std::string& csv_path)
        : sc_core::sc_module(name), csv_path_(csv_path), csv_(csv_path) {
        if (!csv_) {
            failed_ = true;
            ++failed_checks_;
            std::cerr << "ERROR: cannot open CSV log: " << csv_path_ << '\n';
        } else {
            csv_ << "time_ns,test,i_a,i_b,expected_o_sum,expected_o_carry,"
                    "actual_o_sum,actual_o_carry,result\n";
            if (!csv_.good()) {
                failed_ = true;
                ++failed_checks_;
                std::cerr << "ERROR: cannot write CSV header: "
                          << csv_path_ << '\n';
            }
        }
        SC_THREAD(run);
    }

    bool failed() const {
        return failed_;
    }

private:
    struct TestVector {
        const char* name;
        unsigned int a;
        unsigned int b;
    };

    static constexpr std::array<TestVector, 16> test_vectors_{{
        {"four_state_truth_table", 0, 0},
        {"four_state_truth_table", 0, 1},
        {"four_state_truth_table", 0, 2},
        {"four_state_truth_table", 0, 3},
        {"four_state_truth_table", 1, 0},
        {"four_state_truth_table", 1, 1},
        {"four_state_truth_table", 1, 2},
        {"four_state_truth_table", 1, 3},
        {"four_state_truth_table", 2, 0},
        {"four_state_truth_table", 2, 1},
        {"four_state_truth_table", 2, 2},
        {"four_state_truth_table", 2, 3},
        {"four_state_truth_table", 3, 0},
        {"four_state_truth_table", 3, 1},
        {"four_state_truth_table", 3, 2},
        {"four_state_truth_table", 3, 3},
    }};

    inline static const std::array<std::array<sc_dt::sc_logic, 4>, 4>
        sum_oracle_{{
            {{sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_1,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
            {{sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
            {{sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
            {{sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
        }};

    inline static const std::array<std::array<sc_dt::sc_logic, 4>, 4>
        carry_oracle_{{
            {{sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_0,
              sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_0}},
            {{sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_1,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
            {{sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
            {{sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X,
              sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_X}},
        }};

    const std::string csv_path_;
    std::ofstream csv_;
    bool failed_ = false;
    unsigned int checks_ = 0;
    unsigned int failed_checks_ = 0;

    static sc_dt::sc_logic logic_value(unsigned int value) {
        switch (value) {
        case 0:
            return sc_dt::SC_LOGIC_0;
        case 1:
            return sc_dt::SC_LOGIC_1;
        case 2:
            return sc_dt::SC_LOGIC_X;
        default:
            return sc_dt::SC_LOGIC_Z;
        }
    }

    void run() {
        for (const TestVector& vector : test_vectors_) {
            const sc_dt::sc_logic a = logic_value(vector.a);
            const sc_dt::sc_logic b = logic_value(vector.b);
            i_a.write(a);
            i_b.write(b);
            wait(1, sc_core::SC_NS);

            record(vector.name, a, b,
                   sum_oracle_[vector.a][vector.b],
                   carry_oracle_[vector.a][vector.b]);
        }

        if (csv_) {
            csv_ << "# SUMMARY: " << (failed_ ? "FAIL" : "PASS")
                 << " checks=" << checks_ << " failed=" << failed_checks_ << '\n';
            csv_.close();
            if (!csv_.good()) {
                failed_ = true;
                ++failed_checks_;
                std::cerr << "ERROR: failed to finalize CSV log: "
                          << csv_path_ << '\n';
            }
        }

        std::cout << "# SUMMARY: " << (failed_ ? "FAIL" : "PASS")
                  << " checks=" << checks_ << " failed=" << failed_checks_ << '\n'
                  << "CSV log: " << csv_path_ << '\n'
                  << "VCD waveform: half_adder_wave.vcd\n";
        if (failed_) {
            std::cerr << "ERROR: half-adder self-check failed\n";
        }
    }

    void record(const char* test_name,
                const sc_dt::sc_logic& a,
                const sc_dt::sc_logic& b,
                const sc_dt::sc_logic& expected_sum,
                const sc_dt::sc_logic& expected_carry) {
        const sc_dt::sc_logic actual_sum = o_sum.read();
        const sc_dt::sc_logic actual_carry = o_carry.read();
        const bool pass =
            actual_sum == expected_sum && actual_carry == expected_carry;
        const char* result = pass ? "PASS" : "FAIL";
        const double time_ns =
            sc_core::sc_time_stamp().to_seconds() * 1.0e9;

        std::cout << "time=" << time_ns << " ns"
                  << " test=" << test_name
                  << " i_a=" << a.to_char()
                  << " i_b=" << b.to_char()
                  << " expected_o_sum=" << expected_sum.to_char()
                  << " expected_o_carry=" << expected_carry.to_char()
                  << " actual_o_sum=" << actual_sum.to_char()
                  << " actual_o_carry=" << actual_carry.to_char()
                  << " result=" << result << '\n';

        if (csv_) {
            csv_ << time_ns << ',' << test_name << ','
                 << a.to_char() << ',' << b.to_char() << ','
                 << expected_sum.to_char() << ','
                 << expected_carry.to_char() << ','
                 << actual_sum.to_char() << ','
                 << actual_carry.to_char() << ',' << result << '\n';
            if (!csv_.good()) {
                failed_ = true;
                ++failed_checks_;
                std::cerr << "ERROR: failed writing CSV log at check "
                          << checks_ << '\n';
            }
        } else {
            failed_ = true;
        }

        if (!pass) {
            failed_ = true;
            ++failed_checks_;
            std::cerr << "FAIL time=" << time_ns << " ns"
                      << " test=" << test_name
                      << " i_a=" << a.to_char()
                      << " i_b=" << b.to_char()
                      << " expected_o_sum=" << expected_sum.to_char()
                      << " expected_o_carry=" << expected_carry.to_char()
                      << " actual_o_sum=" << actual_sum.to_char()
                      << " actual_o_carry=" << actual_carry.to_char() << '\n';
        }
        ++checks_;
    }
};

#endif
