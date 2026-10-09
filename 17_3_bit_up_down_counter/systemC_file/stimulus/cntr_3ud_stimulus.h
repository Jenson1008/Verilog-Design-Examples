#pragma once

#include <cstddef>
#include <fstream>
#include <iostream>

#include <systemc>

class cntr_3ud_stimulus : public sc_core::sc_module {
public:
    sc_core::sc_out<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_out<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_out<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_in<sc_dt::sc_lv<3>> bin_count{"bin_count"};

    SC_HAS_PROCESS(cntr_3ud_stimulus);

    explicit cntr_3ud_stimulus(sc_core::sc_module_name name)
        : sc_core::sc_module(name) {
        SC_THREAD(run);
    }

    int result() const {
        return completed_ && failures_ == 0 ? 0 : 1;
    }

private:
    void run() {
        log_.open("cntr_3ud.log", std::ios::out | std::ios::trunc);
        if (!log_) {
            std::cerr << "Unable to open cntr_3ud.log for writing.\n";
            failures_ = 1;
            sc_core::sc_stop();
            return;
        }

        log_ << "time_ns,test,reset,up_down,expected,actual,status\n";

        reset.write(sc_dt::SC_LOGIC_1);
        up_down.write(sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check("uninitialized output", sc_dt::sc_lv<3>("XXX"));

        cycle(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0, "synchronous reset");
        cycle(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_1, "reset overrides down");
        check_between_edges(
            sc_dt::SC_LOGIC_0,
            sc_dt::SC_LOGIC_0,
            "deassert reset before rising edge");

        for (int state = 0; state < 8; ++state) {
            cycle(
                sc_dt::SC_LOGIC_0,
                sc_dt::SC_LOGIC_0,
                "increment through modulo-8 states");
        }

        for (int state = 0; state < 8; ++state) {
            cycle(
                sc_dt::SC_LOGIC_0,
                sc_dt::SC_LOGIC_1,
                "decrement through modulo-8 states");
        }

        check_between_edges(
            sc_dt::SC_LOGIC_1,
            sc_dt::SC_LOGIC_0,
            "assert reset away from rising edge");
        cycle(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0, "synchronous reset while counting");
        cycle(sc_dt::SC_LOGIC_X, sc_dt::SC_LOGIC_0, "X reset follows Verilog if semantics");

        check_unknown_clock_edge();
        cycle(sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_0, "increment after X reset");
        cycle(sc_dt::SC_LOGIC_Z, sc_dt::SC_LOGIC_1, "Z reset follows Verilog if semantics");

        cycle(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_X, "reset before unknown direction");
        cycle(sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_X, "X direction follows procedural if else branch");
        cycle(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0, "reset clears unknown count");
        cycle(sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_Z, "Z direction follows procedural if else branch");

        log_ << "# SUMMARY: " << (failures_ == 0 ? "PASS" : "FAIL")
             << " checks=" << check_count_ << " failures=" << failures_ << '\n';
        log_.close();
        if (!log_) {
            std::cerr << "Failed while writing cntr_3ud.log.\n";
            failures_ = 1;
        }
        completed_ = true;
        sc_core::sc_stop();
    }

    void check_between_edges(
        sc_dt::sc_logic next_reset,
        sc_dt::sc_logic direction,
        const char* label) {
        reset.write(next_reset);
        up_down.write(direction);
        wait(1, sc_core::SC_NS);
        check(label, expected_value());
    }

    void cycle(
        sc_dt::sc_logic next_reset,
        sc_dt::sc_logic direction,
        const char* label) {
        reset.write(next_reset);
        up_down.write(direction);
        wait(1, sc_core::SC_NS);

        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        update_expected(next_reset, direction);
        check(label, expected_value());

        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
    }

    void check_unknown_clock_edge() {
        reset.write(sc_dt::SC_LOGIC_1);
        up_down.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);

        clk.write(sc_dt::SC_LOGIC_X);
        wait(1, sc_core::SC_NS);
        update_expected(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0);
        check("0-to-X clock transition triggers Verilog posedge", expected_value());

        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        update_expected(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0);
        check("X-to-1 clock transition triggers Verilog posedge", expected_value());

        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);

        clk.write(sc_dt::SC_LOGIC_Z);
        wait(1, sc_core::SC_NS);
        update_expected(sc_dt::SC_LOGIC_1, sc_dt::SC_LOGIC_0);
        check("0-to-Z clock transition triggers Verilog posedge", expected_value());

        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
    }

    void update_expected(sc_dt::sc_logic reset_value, sc_dt::sc_logic direction) {
        if (reset_value == sc_dt::SC_LOGIC_1) {
            expected_count_ = 0;
            expected_known_ = true;
            return;
        }

        if (!expected_known_) {
            report_oracle_error("counter state is unknown before a non-reset edge");
            return;
        }

        if (direction == sc_dt::SC_LOGIC_1) {
            expected_count_ = (expected_count_ + 7U) & 0x7U;
        } else {
            expected_count_ = (expected_count_ + 1U) & 0x7U;
        }
    }

    sc_dt::sc_lv<3> expected_value() const {
        return expected_known_ ? sc_dt::sc_lv<3>(expected_count_) : sc_dt::sc_lv<3>("XXX");
    }

    void report_oracle_error(const char* message) {
        ++failures_;
        std::cerr << "Reference-model error at " << sc_core::sc_time_stamp()
                  << ": " << message << '\n';
    }

    void check(const char* label, const sc_dt::sc_lv<3>& expected) {
        ++check_count_;
        const sc_dt::sc_lv<3> actual = bin_count.read();
        const bool passed = actual == expected;
        const sc_dt::sc_logic reset_value = reset.read();
        const sc_dt::sc_logic direction = up_down.read();

        log_ << sc_core::sc_time_stamp().to_seconds() * 1.0e9 << ','
             << label << ',' << reset_value.to_char() << ','
             << direction.to_char() << ',' << expected << ',' << actual << ','
             << (passed ? "PASS" : "FAIL") << '\n';

        if (!passed) {
            ++failures_;
            std::cerr << "Counter self-check failed at "
                      << sc_core::sc_time_stamp() << " (" << label
                      << "): reset=" << reset_value
                      << ", up_down=" << direction
                      << ", expected=" << expected
                      << ", actual=" << actual << ".\n";
        }
    }

    std::ofstream log_;
    std::size_t failures_ = 0;
    std::size_t check_count_ = 0;
    unsigned int expected_count_ = 0;
    bool expected_known_ = false;
    bool completed_ = false;
};
