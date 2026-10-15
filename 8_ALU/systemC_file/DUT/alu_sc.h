#ifndef ALU_SC_H
#define ALU_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;
using sc_dt::SC_LOGIC_X;

SC_MODULE(alu) {
    sc_core::sc_in<sc_dt::sc_lv<4>> a{"a"}, b{"b"};
    sc_core::sc_in<sc_dt::sc_lv<2>> select{"select"};
    sc_core::sc_out<sc_dt::sc_logic> zero{"zero"}, carry{"carry"}, sign{"sign"},
        parity{"parity"}, overflow{"overflow"};
    sc_core::sc_out<sc_dt::sc_lv<4>> out{"out"};

    sc_dt::sc_lv<4> previous_out{"XXXX"};
    sc_dt::sc_logic previous_carry{SC_LOGIC_X};

    static bool is_binary(const sc_dt::sc_lv<4>& value) {
        for (int i=0;i<4;++i)
            if (value[i] != SC_LOGIC_0 && value[i] != SC_LOGIC_1) return false;
        return true;
    }
    void evaluate() {
        const auto av=a.read();
        const auto bv=b.read();
        const auto sv=select.read();
        const bool select_known=(sv[1]==SC_LOGIC_0||sv[1]==SC_LOGIC_1) &&
                                (sv[0]==SC_LOGIC_0||sv[0]==SC_LOGIC_1);
        if (select_known && is_binary(av) && is_binary(bv)) {
            const unsigned opcode=sv.to_uint();
            if (opcode<=3) {
                const unsigned x=av.to_uint(), y=bv.to_uint();
                if (opcode==3 && y==0) {
                    previous_out="XXXX";
                    previous_carry=SC_LOGIC_X;
                } else {
                    unsigned result=0;
                    if (opcode==0) result=x+y;
                    else if (opcode==1) result=(x-y)&31U;
                    else if (opcode==2) result=(x*y)&31U;
                    else result=x/y;
                    previous_carry=(result&16U)?SC_LOGIC_1:SC_LOGIC_0;
                    for(int bit=0;bit<4;++bit)
                        previous_out[bit]=((result>>bit)&1U)?SC_LOGIC_1:SC_LOGIC_0;
                }
            }
        } else if (select_known) {
            // Verilog arithmetic yields X when an arithmetic operand contains X/Z.
            previous_out="XXXX";
            previous_carry=SC_LOGIC_X;
        }
        // An unmatched four-state case leaves the result registers unchanged.
        sc_dt::sc_logic or_reduction=SC_LOGIC_0, xor_reduction=SC_LOGIC_0;
        for(int bit=0;bit<4;++bit) {
            or_reduction=or_reduction|previous_out[bit];
            xor_reduction=xor_reduction^previous_out[bit];
        }
        const auto zero_value=~or_reduction;
        const auto sign_value=previous_out[3];
        const auto parity_value=~xor_reduction;
        const auto overflow_value=(av[3]&bv[3]&~previous_out[3]) |
                                  (~av[3]&~bv[3]&previous_out[3]);
        out.write(previous_out); carry.write(previous_carry); zero.write(zero_value);
        sign.write(sign_value); parity.write(parity_value); overflow.write(overflow_value);
    }
    SC_CTOR(alu) {
        SC_METHOD(evaluate);
        // RTL says always @(a or b), intentionally excluding select.
        sensitive << a << b;
    }
};

#endif
