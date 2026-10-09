# 基本说明

`cntr_3ud` 是一个 3 位模 8 加减计数器。每次 Verilog 正边沿触发时，高电平有效的同步复位 `reset` 清零计数；否则 `up_down = 0` 加一、`up_down = 1` 减一，溢出时按 3 位宽度回绕。

SystemC DUT 使用 `sc_logic` 和 `sc_lv<3>` 保留四态信号。RTL 未初始化 `bin_count`，所以有效时钟边沿采样到 `reset = 1` 前输出为未知。`up_down` 在过程式 `if` 中为 `1` 时减一；为 `0`、`X` 或 `Z` 时均走 `else` 分支加一。未知复位在 procedural `if` 中走非复位分支。

原 Verilog testbench 在时钟上升沿同时驱动方向输入并更新参考计数，且在 25 ns 的上升沿撤销复位，存在调度竞态。本 SystemC 测试在边沿前驱动输入并于状态更新后检查；不声称复现原 testbench 的随机序列或证明形式等价。

接口快照见同目录的 `interface.json`；未确定事项及端口契约见 `../../interface/interface.json`。
