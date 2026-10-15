#ifndef DUALPORT_RAM_SC_H
#define DUALPORT_RAM_SC_H

#include <systemc>

SC_MODULE(dualport_ram) {
    sc_core::sc_in<sc_dt::sc_lv<128>> data_in_a{"data_in_a"}, data_in_b{"data_in_b"};
    sc_core::sc_in<sc_dt::sc_logic> wr_en_a{"wr_en_a"}, wr_en_b{"wr_en_b"}, clk{"clk"};
    sc_core::sc_in<sc_dt::sc_lv<3>> addr_a{"addr_a"}, addr_b{"addr_b"};
    sc_core::sc_out<sc_dt::sc_lv<128>> data_out_a{"data_out_a"}, data_out_b{"data_out_b"};

    SC_CTOR(dualport_ram) {
        SC_METHOD(edge);
        sensitive << clk.pos();
        dont_initialize();
    }

public: // Exposed only for waveform tracing; these are not DUT ports.
    sc_core::sc_signal<sc_dt::sc_lv<128>> ram_vector[8];

private:
    static sc_dt::sc_lv<128> unknown_word() {
        sc_dt::sc_lv<128> value;
        for (unsigned i = 0; i < 128; ++i) value[i] = sc_dt::SC_LOGIC_X;
        return value;
    }

    sc_dt::sc_lv<128> read_word(const sc_dt::sc_lv<3>& address) const {
        if (!address.is_01()) return unknown_word();
        return ram_vector[address.to_uint()].read();
    }

    void edge() {
        const auto a_addr = addr_a.read();
        const auto b_addr = addr_b.read();
        const bool a_write = wr_en_a.read() == sc_dt::SC_LOGIC_1;
        const bool b_write = wr_en_b.read() == sc_dt::SC_LOGIC_1;

        // In each RTL process, read RHS values are sampled before the
        // nonblocking memory writes commit. Read both ports from old state.
        if (!a_write) data_out_a.write(read_word(a_addr));
        if (!b_write) data_out_b.write(read_word(b_addr));

        if (a_write && b_write) {
            if (a_addr.is_01() && b_addr.is_01()) {
                const auto ai = a_addr.to_uint();
                const auto bi = b_addr.to_uint();
                if (ai == bi) {
                    // The source has two competing NBAs to this word. Per
                    // the agreed constrained contract, retain no priority:
                    // make only this collided word unknown.
                    ram_vector[ai].write(unknown_word());
                } else {
                    ram_vector[ai].write(data_in_a.read());
                    ram_vector[bi].write(data_in_b.read());
                }
            } else {
                // X/Z addresses are outside the verified input space.
                // Avoid inventing a winner when the write targets cannot
                // be established.
            }
        } else if (a_write && a_addr.is_01()) {
            ram_vector[a_addr.to_uint()].write(data_in_a.read());
        } else if (b_write && b_addr.is_01()) {
            ram_vector[b_addr.to_uint()].write(data_in_b.read());
        }
    }
};

#endif
