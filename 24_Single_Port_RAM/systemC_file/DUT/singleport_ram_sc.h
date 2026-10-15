#ifndef SINGLEPORT_RAM_SC_H
#define SINGLEPORT_RAM_SC_H

#include <systemc>

SC_MODULE(singleport_ram) {
    sc_core::sc_in<sc_dt::sc_lv<128>> data_in{"data_in"};
    sc_core::sc_in<sc_dt::sc_logic> wr_en{"wr_en"}, clk{"clk"};
    sc_core::sc_in<sc_dt::sc_lv<3>> addr{"addr"};
    sc_core::sc_out<sc_dt::sc_lv<128>> data_out{"data_out"};

    SC_CTOR(singleport_ram) {
        SC_METHOD(write_and_address);
        sensitive << clk.pos();
        dont_initialize();
        SC_METHOD(read_mux);
        sensitive << read_addr;
        for (unsigned i = 0; i < 8; ++i) sensitive << ram_vector[i];
    }

private:
    sc_core::sc_signal<sc_dt::sc_lv<128>> ram_vector[8];
    sc_core::sc_signal<sc_dt::sc_lv<3>> read_addr{"read_addr"};

    void write_and_address() {
        const auto address = addr.read();
        if (wr_en.read() == sc_dt::SC_LOGIC_1 && address.is_01())
            ram_vector[address.to_uint()].write(data_in.read());
        read_addr.write(address);
    }
    void read_mux() {
        const auto address = read_addr.read();
        if (!address.is_01()) { data_out.write(sc_dt::sc_lv<128>(sc_dt::Log_X)); return; }
        data_out.write(ram_vector[address.to_uint()].read());
    }
};

#endif
