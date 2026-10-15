#ifndef PIPELINE_SC_H
#define PIPELINE_SC_H

#include <systemc>

#include <cstdint>

// Cycle-level model of the RTL module pipeline.
//
// The original design uses nonblocking assignments in separate posedge
// processes. This method snapshots all old register values, computes each
// process's right-hand sides from that snapshot, then schedules the next
// register values together. This preserves the parallel nonblocking update
// semantics without combining any state dependencies.
SC_MODULE(pipeline) {
    sc_core::sc_in<sc_dt::sc_lv<64>> x;
    sc_core::sc_in<sc_dt::sc_lv<64>> y;
    sc_core::sc_in<sc_dt::sc_logic> clk;
    sc_core::sc_in<sc_dt::sc_logic> reset;
    sc_core::sc_out<sc_dt::sc_lv<128>> prod;

    // Internal register signals are exposed only for waveform tracing.
    sc_core::sc_signal<sc_dt::sc_lv<64>> xtemp_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> ytemp_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temp1_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temp2_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temp3_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temp4_reg;
    sc_core::sc_signal<sc_dt::sc_logic> tempc1_reg;
    sc_core::sc_signal<sc_dt::sc_logic> tempc2_reg;
    sc_core::sc_signal<sc_dt::sc_logic> has_reg;
    sc_core::sc_signal<sc_dt::sc_logic> haca_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temps2_reg;
    sc_core::sc_signal<sc_dt::sc_lv<64>> temps3_reg;

    template <int WIDTH>
    struct AddResult {
        sc_dt::sc_lv<WIDTH> sum;
        sc_dt::sc_logic carry;
    };

    SC_CTOR(pipeline) {
        SC_METHOD(clock_edge);
        sensitive << clk.pos();
        dont_initialize();
    }

    template <int OUT_WIDTH, int IN_WIDTH>
    static sc_dt::sc_lv<OUT_WIDTH> slice(
        const sc_dt::sc_lv<IN_WIDTH>& value, int lsb) {
        sc_dt::sc_lv<OUT_WIDTH> result;
        for (int bit = 0; bit < OUT_WIDTH; ++bit) {
            result[bit] = value[lsb + bit];
        }
        return result;
    }

    template <int WIDTH>
    static AddResult<WIDTH> add_ripple(
        const sc_dt::sc_lv<WIDTH>& a,
        const sc_dt::sc_lv<WIDTH>& b,
        const sc_dt::sc_logic& carry_in) {
        AddResult<WIDTH> result;
        sc_dt::sc_logic carry = carry_in;

        for (int bit = 0; bit < WIDTH; ++bit) {
            const sc_dt::sc_logic a_bit = a[bit];
            const sc_dt::sc_logic b_bit = b[bit];
            result.sum[bit] = a_bit ^ b_bit ^ carry;

            // Keep the source RTL's three-product carry expression. Replacing
            // it by an algebraically equivalent XOR form can change X
            // pessimism in four-state simulation.
            carry = (a_bit & b_bit) |
                    (b_bit & carry) |
                    (carry & a_bit);
        }
        result.carry = carry;
        return result;
    }

    static sc_dt::sc_lv<4> multiply_2x2(
        const sc_dt::sc_lv<2>& a,
        const sc_dt::sc_lv<2>& b) {
        const sc_dt::sc_logic p00 = a[0] & b[0];
        const sc_dt::sc_logic p10 = a[1] & b[0];
        const sc_dt::sc_logic p01 = a[0] & b[1];
        const sc_dt::sc_logic p11 = a[1] & b[1];
        const sc_dt::sc_logic carry1 = p10 & p01;

        sc_dt::sc_lv<4> result(sc_dt::SC_LOGIC_0);
        result[0] = p00;
        result[1] = p10 ^ p01;
        result[2] = carry1 ^ p11;
        result[3] = carry1 & p11;
        return result;
    }

    // Mirrors the recursively decomposed vdcmul_4b/8b/16b/32b RTL hierarchy.
    // The recursive gate/add implementation retains four-state behavior.
    template <int WIDTH>
    static sc_dt::sc_lv<2 * WIDTH> multiply_recursive(
        const sc_dt::sc_lv<WIDTH>& a,
        const sc_dt::sc_lv<WIDTH>& b) {
        if constexpr (WIDTH == 2) {
            return multiply_2x2(a, b);
        } else {
            static_assert((WIDTH % 2) == 0, "multiplier width must be even");
            constexpr int HALF = WIDTH / 2;

            const sc_dt::sc_lv<HALF> a_low = slice<HALF>(a, 0);
            const sc_dt::sc_lv<HALF> a_high = slice<HALF>(a, HALF);
            const sc_dt::sc_lv<HALF> b_low = slice<HALF>(b, 0);
            const sc_dt::sc_lv<HALF> b_high = slice<HALF>(b, HALF);

            const sc_dt::sc_lv<WIDTH> pp1 =
                multiply_recursive<HALF>(a_low, b_low);
            const sc_dt::sc_lv<WIDTH> pp2 =
                multiply_recursive<HALF>(a_low, b_high);
            const sc_dt::sc_lv<WIDTH> pp3 =
                multiply_recursive<HALF>(a_high, b_low);
            const sc_dt::sc_lv<WIDTH> pp4 =
                multiply_recursive<HALF>(a_high, b_high);

            const AddResult<WIDTH> cross =
                add_ripple(pp2, pp3, sc_dt::SC_LOGIC_0);

            sc_dt::sc_lv<WIDTH> pp1_high_operand(sc_dt::SC_LOGIC_0);
            for (int bit = 0; bit < HALF; ++bit) {
                pp1_high_operand[bit] = pp1[HALF + bit];
            }
            const AddResult<WIDTH> add_low_high =
                add_ripple(cross.sum, pp1_high_operand, sc_dt::SC_LOGIC_0);

            const sc_dt::sc_logic carry_sum =
                cross.carry ^ add_low_high.carry;
            const sc_dt::sc_logic carry_carry =
                cross.carry & add_low_high.carry;

            sc_dt::sc_lv<WIDTH> upper_operand(sc_dt::SC_LOGIC_0);
            for (int bit = 0; bit < HALF; ++bit) {
                upper_operand[bit] = add_low_high.sum[HALF + bit];
            }
            upper_operand[HALF] = carry_sum;
            upper_operand[HALF + 1] = carry_carry;
            const AddResult<WIDTH> upper =
                add_ripple(pp4, upper_operand, sc_dt::SC_LOGIC_0);

            sc_dt::sc_lv<2 * WIDTH> result(sc_dt::SC_LOGIC_0);
            for (int bit = 0; bit < HALF; ++bit) {
                result[bit] = pp1[bit];
                result[HALF + bit] = add_low_high.sum[bit];
            }
            for (int bit = 0; bit < WIDTH; ++bit) {
                result[WIDTH + bit] = upper.sum[bit];
            }
            return result;
        }
    }

    static sc_dt::sc_lv<128> assemble_product(
        const sc_dt::sc_lv<64>& upper,
        const sc_dt::sc_lv<64>& middle,
        const sc_dt::sc_lv<64>& lower) {
        sc_dt::sc_lv<128> result;
        for (int bit = 0; bit < 32; ++bit) {
            result[bit] = lower[bit];
            result[32 + bit] = middle[bit];
        }
        for (int bit = 0; bit < 64; ++bit) {
            result[64 + bit] = upper[bit];
        }
        return result;
    }

    void clock_edge() {
        // Snapshot all register values before calculating any next values.
        const sc_dt::sc_lv<64> old_xtemp = xtemp_reg.read();
        const sc_dt::sc_lv<64> old_ytemp = ytemp_reg.read();
        const sc_dt::sc_lv<64> old_temp1 = temp1_reg.read();
        const sc_dt::sc_lv<64> old_temp2 = temp2_reg.read();
        const sc_dt::sc_lv<64> old_temp3 = temp3_reg.read();
        const sc_dt::sc_lv<64> old_temp4 = temp4_reg.read();
        const sc_dt::sc_logic old_tempc1 = tempc1_reg.read();
        const sc_dt::sc_logic old_tempc2 = tempc2_reg.read();
        const sc_dt::sc_logic old_has = has_reg.read();
        const sc_dt::sc_logic old_haca = haca_reg.read();
        const sc_dt::sc_lv<64> old_temps2 = temps2_reg.read();

        const sc_dt::sc_lv<32> x_low = slice<32>(old_xtemp, 0);
        const sc_dt::sc_lv<32> x_high = slice<32>(old_xtemp, 32);
        const sc_dt::sc_lv<32> y_low = slice<32>(old_ytemp, 0);
        const sc_dt::sc_lv<32> y_high = slice<32>(old_ytemp, 32);
        const sc_dt::sc_lv<64> pp1 =
            multiply_recursive<32>(x_low, y_low);
        const sc_dt::sc_lv<64> pp2 =
            multiply_recursive<32>(x_low, y_high);
        const sc_dt::sc_lv<64> pp3 =
            multiply_recursive<32>(x_high, y_low);
        const sc_dt::sc_lv<64> pp4 =
            multiply_recursive<32>(x_high, y_high);

        const AddResult<64> sum1 =
            add_ripple(old_temp2, old_temp3, sc_dt::SC_LOGIC_0);

        sc_dt::sc_lv<64> temp1_high_operand(sc_dt::SC_LOGIC_0);
        for (int bit = 0; bit < 32; ++bit) {
            temp1_high_operand[bit] = old_temp1[32 + bit];
        }
        const AddResult<64> sum2 =
            add_ripple(sum1.sum, temp1_high_operand, sc_dt::SC_LOGIC_0);

        sc_dt::sc_lv<64> sum3_operand(sc_dt::SC_LOGIC_0);
        for (int bit = 0; bit < 32; ++bit) {
            sum3_operand[bit] = old_temps2[32 + bit];
        }
        sum3_operand[32] = old_has;
        sum3_operand[33] = old_haca;
        const AddResult<64> sum3 =
            add_ripple(old_temp4, sum3_operand, sc_dt::SC_LOGIC_0);

        const sc_dt::sc_lv<64> next_xtemp =
            reset.read() == sc_dt::SC_LOGIC_1
                ? sc_dt::sc_lv<64>(sc_dt::SC_LOGIC_0)
                : x.read();
        const sc_dt::sc_lv<64> next_ytemp =
            reset.read() == sc_dt::SC_LOGIC_1
                ? sc_dt::sc_lv<64>(sc_dt::SC_LOGIC_0)
                : y.read();
        const sc_dt::sc_logic next_has = old_tempc1 ^ old_tempc2;
        const sc_dt::sc_logic next_haca = old_tempc1 & old_tempc2;

        xtemp_reg.write(next_xtemp);
        ytemp_reg.write(next_ytemp);
        temp1_reg.write(pp1);
        temp2_reg.write(pp2);
        temp3_reg.write(pp3);
        temp4_reg.write(pp4);
        tempc1_reg.write(sum1.carry);
        tempc2_reg.write(sum2.carry);
        has_reg.write(next_has);
        haca_reg.write(next_haca);
        temps2_reg.write(sum2.sum);
        temps3_reg.write(sum3.sum);

        // The RTL continuously concatenates these register slices.
        prod.write(assemble_product(sum3.sum, sum2.sum, pp1));
    }
};

#endif  // PIPELINE_SC_H
