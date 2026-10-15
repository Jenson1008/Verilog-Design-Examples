#ifndef COUNTER4BIT_SC_H
#define COUNTER4BIT_SC_H

#include <systemc>

SC_MODULE(counter4bit) {
    sc_core::sc_in<bool> reset{"reset"}, clk{"clk"};
    sc_core::sc_in<sc_dt::sc_logic> load{"load"}, enable{"enable"};
    sc_core::sc_out<sc_dt::sc_lv<4>> count_out{"count_out"}, count{"count"};
    sc_dt::sc_lv<4> count_out_reg;
    sc_dt::sc_lv<4> count_reg;

    static unsigned following(unsigned value) {
        switch (value) {
        case 8: return 7; case 7: return 11; case 11: return 4; case 4: return 9;
        case 9: return 2; case 2: return 5; case 5: return 12; case 12: return 6;
        case 6: return 3; case 3: return 15; case 15: return 1; case 1: return 14;
        case 14: return 13; case 13: return 8; default: return 8;
        }
    }
    void update() {
        const bool clock_edge = clk.event() && clk.read();
        const sc_dt::sc_lv<4> old_count_out = count_out_reg;
        const sc_dt::sc_lv<4> old_count = count_reg;
        sc_dt::sc_lv<4> next_count_out = reset.read()
            ? sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(8)) : old_count;
        sc_dt::sc_lv<4> next_count = old_count;

        // This is a separate posedge-only RTL process and has no reset branch.
        if (clock_edge) {
            if (load.read() == sc_dt::SC_LOGIC_1 && enable.read() == sc_dt::SC_LOGIC_1) {
                next_count = old_count_out.is_01()
                    ? sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(following(old_count_out.to_uint())))
                    : sc_dt::sc_lv<4>("XXXX");
            } else if (load.read() == sc_dt::SC_LOGIC_0) {
                next_count = sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(8));
            } else if (enable.read() == sc_dt::SC_LOGIC_0) {
                next_count_out = old_count;
            }
        }
        count_reg = next_count;
        count_out_reg = next_count_out;
        count.write(count_reg);
        count_out.write(count_out_reg);
    }
    SC_CTOR(counter4bit) {
        SC_METHOD(update);
        sensitive << clk.pos() << reset.pos();
        dont_initialize();
    }
};

#endif
