#ifndef UNIV_SHIFT_SC_H
#define UNIV_SHIFT_SC_H

#include <systemc>

SC_MODULE(univ_shift) {
    sc_core::sc_in<sc_dt::sc_lv<16>> data_in{"data_in"};
    sc_core::sc_in<sc_dt::sc_lv<2>> control{"control"};
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_in<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_out<sc_dt::sc_lv<16>> data_out{"data_out"};

    SC_CTOR(univ_shift) {
        SC_METHOD(sequential);
        sensitive << clk.pos() << reset.neg();
        dont_initialize();
    }

private:
    void sequential() {
        // RTL sensitivity is posedge clk OR negedge reset, but the reset
        // condition itself is active-high. Preserve that unusual event rule.
        if (reset.read() == sc_dt::SC_LOGIC_1) {
            data_out.write(sc_dt::sc_lv<16>(0));
            return;
        }
        const auto mode = control.read();
        if (!mode.is_01()) return; // A non-binary case item matches no RTL arm.
        switch (mode.to_uint()) {
        case 0: break; // hold
        case 1: data_out.write(data_in.read() << 1); break;
        case 2: data_out.write(data_in.read() >> 1); break;
        case 3: data_out.write(data_in.read()); break;
        default: break;
        }
    }
};

#endif
