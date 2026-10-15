# mux_case

## 设计行为

`mux_case` 在四路两位数据 `in0` 至 `in3` 中选择一路送到 `out`：`select` 为 `00`、`01`、`10`、`11` 时，分别选择 `in0`、`in1`、`in2`、`in3`。

此设计的更新触发与普通“数据一变就跟随所选输入”的理想 mux 不同：RTL 的 `always` 和 SystemC 的 `SC_METHOD` 都只对 `select` 敏感。改变 `select` 会重新读取当前所选数据并更新 `out`；若 `select` 保持不变而某个 `in*` 单独变化，过程不会运行，`out` 因而保持上一次写出的值，直到 `select` 再次变化。没有匹配 case 分支时也没有默认赋值。
