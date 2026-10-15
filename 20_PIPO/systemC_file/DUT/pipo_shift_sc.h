#ifndef PIPO_SHIFT_SC_H
#define PIPO_SHIFT_SC_H

#include <systemc>

SC_MODULE(pipo_shift) {
    sc_core::sc_in<sc_dt::sc_lv<16>> data_in{"data_in"};
    sc_core::sc_in<sc_dt::sc_logic> load{"load"};
    sc_core::sc_in<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_lv<2>> shift_en{"shift_en"};
    sc_core::sc_out<sc_dt::sc_lv<16>> data_out{"data_out"};
    sc_core::sc_signal<sc_dt::sc_lv<16>> temp_data{"temp_data"};

    void operate() {
        if (reset.read()) {
            data_out.write(sc_dt::sc_lv<16>("0000000000000000"));
        } else if (load.read() == sc_dt::SC_LOGIC_1) {
            const sc_dt::sc_lv<16> old_temp = temp_data.read();
            temp_data.write(data_in.read());
            const sc_dt::sc_lv<2> mode = shift_en.read();
            if (mode == sc_dt::sc_lv<2>("00")) data_out.write(old_temp << 1);
            else if (mode == sc_dt::sc_lv<2>("01")) data_out.write(old_temp >> 1);
            else if (mode == sc_dt::sc_lv<2>("10")) data_out.write(old_temp << 1); // Verilog <<< on unsigned packed vector.
            else if (mode == sc_dt::sc_lv<2>("11")) {
                sc_dt::sc_lv<16> shifted;
                shifted[15] = old_temp[15];
                for (int bit = 14; bit >= 0; --bit) shifted[bit] = old_temp[bit + 1];
                data_out.write(shifted);
            }
            // An X/Z case selector matches no case item; RTL retains data_out.
        }
    }
    SC_CTOR(pipo_shift) {
        SC_METHOD(operate);
        sensitive << clk.pos();
        dont_initialize();
    }
};

#endif
