# Basic Description

`full_half_add_1bit` is a combinational 1-bit full adder with inputs `i_a`, `i_b`, and `i_cin`, and outputs `o_sum` and `o_carry`. It is implemented with two half adders; the sum and carry represent the binary addition of the three inputs.

## Unknowns

- The `bool` interface models only 0/1; Verilog `X/Z` propagation is not covered.
- No clock, reset, or stateful behavior is specified for this DUT.
