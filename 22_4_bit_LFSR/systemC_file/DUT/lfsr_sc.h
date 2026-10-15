#ifndef LFSR_SC_H
#define LFSR_SC_H

#include <systemc>

SC_MODULE(lfsr) {
    sc_core::sc_in<sc_dt::sc_logic> reset{"reset"};
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"};
    sc_core::sc_out<sc_dt::sc_lv<4>> data_out{"data_out"};

    SC_CTOR(lfsr) {
        SC_METHOD(step);
        sensitive << clk.pos();
        dont_initialize();
    }

private:
    void step() {
        const auto state = data_out.read();
        if (reset.read() == sc_dt::SC_LOGIC_1 ||
            (state.is_01() && state.to_uint() == 0)) {
            data_out.write(sc_dt::sc_lv<4>(1));
        } else {
            sc_dt::sc_lv<4> next;
            next[3] = state[1] ^ state[0];
            next[2] = state[3];
            next[1] = state[2];
            next[0] = state[1];
            data_out.write(next);
        }
    }
};

#endif
