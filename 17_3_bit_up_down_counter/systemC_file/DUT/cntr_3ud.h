#pragma once

#include <systemc>

SC_MODULE(cntr_3ud) {
    sc_core::sc_in<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_in<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_out<sc_dt::sc_lv<3>> bin_count{"bin_count"};

    SC_CTOR(cntr_3ud) {
        SC_METHOD(observe_clock);
        sensitive << clk;
    }

private:
    static bool is_posedge(sc_dt::sc_logic previous, sc_dt::sc_logic current) {
        if (previous == sc_dt::SC_LOGIC_0) {
            return current != sc_dt::SC_LOGIC_0;
        }
        return (previous == sc_dt::SC_LOGIC_X
                || previous == sc_dt::SC_LOGIC_Z)
            && current == sc_dt::SC_LOGIC_1;
    }

    static sc_dt::sc_lv<3> add_one(const sc_dt::sc_lv<3>& current) {
        sc_dt::sc_lv<3> result;
        sc_dt::sc_logic carry = sc_dt::SC_LOGIC_1;
        for (int bit = 0; bit < 3; ++bit) {
            result[bit] = current[bit] ^ carry;
            carry = current[bit] & carry;
        }
        return result;
    }

    static sc_dt::sc_lv<3> subtract_one(const sc_dt::sc_lv<3>& current) {
        sc_dt::sc_lv<3> result;
        sc_dt::sc_logic borrow = sc_dt::SC_LOGIC_1;
        for (int bit = 0; bit < 3; ++bit) {
            result[bit] = current[bit] ^ borrow;
            borrow = (~current[bit]) & borrow;
        }
        return result;
    }

    void observe_clock() {
        const sc_dt::sc_logic current_clock = clk.read();
        if (clk.event() && is_posedge(previous_clock_, current_clock)) {
            update_count();
        }
        previous_clock_ = current_clock;
    }

    void update_count() {
        if (reset.read() == sc_dt::SC_LOGIC_1) {
            bin_count.write(sc_dt::sc_lv<3>("000"));
            return;
        }

        const sc_dt::sc_lv<3> current = bin_count.read();
        const sc_dt::sc_lv<3> incremented = add_one(current);
        const sc_dt::sc_lv<3> decremented = subtract_one(current);

        if (up_down.read() == sc_dt::SC_LOGIC_1) {
            bin_count.write(decremented);
        } else {
            bin_count.write(incremented);
        }
    }

    sc_dt::sc_logic previous_clock_ = sc_dt::SC_LOGIC_X;
};
