#include "DUT/full_half_add_1bit.h"
#include "stimulus/full_half_add_1bit_stimulus.h"
#include "stimulus/full_half_add_1bit_testbench.h"

int sc_main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    return run_full_half_add_1bit_testbench(
        "full_half_add_1bit.csv", "full_half_add_1bit_wave", true);
}
