# 全加器设计说明

## 设计功能

顶层模块 `full_half_add_1bit` 使用两个 `half_adder` 组成一个 1 位全加器。输入为 `i_a`、`i_b` 和 `i_cin`；输出 `o_sum` 表示本位和，`o_carry` 表示进位。RTL 以异或、与和或门描述该组合逻辑。

## 建模说明

SystemC DUT 保留 `full_half_add_1bit` 与两个 `half_adder` 的层次和内部信号。端口及内部逻辑使用 `sc_logic`，保留 0、1、X、Z 四态；组合过程由输入变化触发，仿真调度按 SystemC delta-cycle 更新。设计没有时钟、复位或显式延迟。

自检对全部 8 种二值输入组合进行穷举，期望值由独立的整数加法计算；另外使用定向 X/Z 输入检查四态传播。每组输入后等待 1 ns 再采样，并将逐项结果同时写入 CSV 和 terminal，再记录汇总及 VCD 波形。

## Unknown 与限制

RTL 未声明未定义或未知的功能行为；`declared_unknowns` 因此为空。原始 Verilog testbench 的随机种子未固定且只打印比较结果，本 SystemC 自检改用可重复的穷举测试，不修改原始 RTL 或 testbench。有限自检和成功仿真不构成形式等价证明。
