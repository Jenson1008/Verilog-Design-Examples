#ifndef SEQMULT_SC_H
#define SEQMULT_SC_H

#include <systemc>

SC_MODULE(seqmult) {
    sc_core::sc_in<sc_dt::sc_lv<8>> in_a{"in_a"}, in_b{"in_b"};
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"}, load{"load"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> out_valid{"out_valid"};
    sc_core::sc_out<sc_dt::sc_lv<16>> out_prod{"out_prod"};

    SC_CTOR(seqmult) {
        SC_METHOD(step);
        sensitive << clk.pos() << reset.pos();
        dont_initialize();
    }

private:
    sc_dt::sc_lv<16> partial[8];

    void step() {
        if (reset.read() == sc_dt::SC_LOGIC_1) {
            out_prod.write(sc_dt::sc_lv<16>(sc_dt::SC_LOGIC_Z));
            out_valid.write(sc_dt::SC_LOGIC_0);
            return;
        }
        if (clk.read() != sc_dt::SC_LOGIC_1 || load.read() != sc_dt::SC_LOGIC_1) return;
        sc_dt::sc_lv<16> sum = 0;
        bool known = true;
        unsigned numeric_sum = 0;
        for (const auto& term : partial) {
            if (!term.is_01()) { known = false; break; }
            numeric_sum += term.to_uint();
        }
        if (known) sum = numeric_sum & 0xffffu;
        else sum = "XXXXXXXXXXXXXXXX";
        out_prod.write(sum); // Nonblocking RTL uses the previous partial products.
        out_valid.write(sc_dt::SC_LOGIC_1);
        sc_dt::sc_lv<16> multiplier = 0;
        multiplier.range(7, 0) = in_b.read();
        for (unsigned i = 0; i < 8; ++i) {
            if (in_a.read()[i] == sc_dt::SC_LOGIC_1) partial[i] = multiplier << i;
            else partial[i] = 0;
        }
    }
};

#endif
